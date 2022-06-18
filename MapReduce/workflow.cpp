// workflow.cpp
// Todd Hricik
// CSE687 Object Oriented Design
// 
// April 28, 2022 - Project 1
// 
// Workflow class implementation
//
// May 12, 2022 - Project 2
//	Updated for project 2 to load map and reduce functions
//	from DLLs
// 
// June 2, 2022 - Project 3
//	Updated to run map and reduce processes via threads
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <regex>
#include <algorithm> 
#include <boost\filesystem.hpp>
#include <boost\filesystem\fstream.hpp>
#include <boost\log\trivial.hpp>
#include "workflow.h"

#include "windows.h"

// Constructor that creates boost::filesystem::path objects for the input directory, intermediate files directory and the output directory
Workflow::Workflow(std::string input_dir_arg,
	std::string inter_dir_arg,
	std::string output_dir_arg,
	int num_mappers,
	int num_reducers,
	int controller_port,
	std::vector<std::string> stub_endpoints)
{
	BOOST_LOG_TRIVIAL(debug) << "Debug in Workflow constructor: Entering constructor.";

	this->num_mappers_ = num_mappers;
	this->num_reducers_ = num_reducers;


	// Start the controller socket and validate the stubs
	startController(controller_port, stub_endpoints);

	// Set the input, temp and output directories
	setInputDirectory(input_dir_arg);
	setTempDirectory(inter_dir_arg);
	setOutputDirectory(output_dir_arg);
}

// Destructor
Workflow::~Workflow()
{
	// Delete intermediate files
	BOOST_LOG_TRIVIAL(info) << "Removing intermediate files...";
	for (boost::filesystem::directory_iterator end_dir_it, it(this->intermediate_dir_); it != end_dir_it; ++it) {
		boost::filesystem::remove_all(it->path());
	}

	// Shutdown the socket
	this->controller_->stop();

}

void Workflow::startController(int port, std::vector<std::string> stub_endpoints) {

	// Start the controller socket
	this->endpoint_ = new MsgPassingCommunication::EndPoint("localhost", port);
	try {
		this->controller_ = new MsgPassingCommunication::Comm(*this->endpoint_, "controller");
		this->controller_->start();
		BOOST_LOG_TRIVIAL(info) << "Started controller at localhost:" << port;
	}
	catch (const std::exception&) {
		// Failed to start controller
		BOOST_LOG_TRIVIAL(fatal) << "Unable to start controller at localhost:" << port;
		exit(1);
	}

	// Validate endpoints for the stubs, try to connect to them
	std::string stub_host;
	int stub_port;
	MsgPassingCommunication::EndPoint stub_endpoint;
	bool can_connect;
	for (std::string stub: stub_endpoints) {
		// Create endpoint
		size_t pos = stub.find(':');
		if (pos != std::string::npos) {
			stub_host = stub.substr(0, pos);
			// Convert port to an integer
			try {
				stub_port = std::stoi(stub.substr(pos + 1));

			} catch (const std::exception&) {
				// Port is not an integer
				BOOST_LOG_TRIVIAL(fatal) << "Invalid stub endpoint " << stub;
				exit(1);
			}
			stub_endpoint = MsgPassingCommunication::EndPoint(stub_host, stub_port);


			// Check controller can connect to the endpoint
			can_connect = this->controller_->connect(stub_endpoint);
			// Could not connect to the stub
			if (!can_connect) {
				BOOST_LOG_TRIVIAL(fatal) << "Failed to connect to endpoint at " << stub;
				exit(1);
			}
			else {
				BOOST_LOG_TRIVIAL(info) << "Connected to endpoint at " << stub;
			}
			
			this->stubs_.push_back(MsgPassingCommunication::EndPoint(stub_host, stub_port));
		}
		else {
			BOOST_LOG_TRIVIAL(fatal) << "Invalid stub endpoint " << stub;
			exit(1);
		}
	}

}

// Creates a map message to pass to an endpoint
MsgPassingCommunication::Message Workflow::createMapMessage(const MsgPassingCommunication::EndPoint& destination,
	const std::vector<boost::filesystem::path>& files,
	int partitions)

{
	MsgPassingCommunication::Message msg;
	msg.to(destination);
	msg.from(*(this->endpoint_));
	msg.command("run_map");
	msg.attribute("partitions", std::to_string(partitions));
	msg.attribute("output_directory", boost::filesystem::absolute(this->intermediate_dir_).string());
	

	std::stringstream ss;
	for (auto it = files.begin(); it != files.end(); it++) {
		if (it != files.begin()) {
			ss << ",";
		}
		ss << boost::filesystem::absolute(*it).string();
	}
	msg.attribute("input_files", ss.str());
	return msg;
}

// Creates a reduce message to pass to an endpoint
MsgPassingCommunication::Message Workflow::createReduceMessage(const MsgPassingCommunication::EndPoint& destination,
	const std::vector<boost::filesystem::path>& files,
	const boost::filesystem::path& output_directory, 
	int partition)
{
	MsgPassingCommunication::Message msg;
	msg.to(destination);
	msg.from(*(this->endpoint_));
	msg.command("run_reduce");
	msg.attribute("partition", std::to_string(partition));
	msg.attribute("output_directory", boost::filesystem::absolute(output_directory).string());


	std::stringstream ss;
	for (auto it = files.begin(); it != files.end(); it++) {
		if (it != files.begin()) {
			ss << ",";
		}
		ss << boost::filesystem::absolute(*it).string();
	}
	msg.attribute("input_files", ss.str());
	return msg;
}


// Validate, create directories
// Set all of the paths that will be used as data directories
void Workflow::setInputDirectory(std::string s)
{
	// Create boost path object for targetDir containing input files
	boost::filesystem::path input_dir(s);

	// If path a is a directory, then assign the path to targetDir data member
	if (boost::filesystem::is_directory(input_dir))
	{
		this->target_dir_ = s;
		BOOST_LOG_TRIVIAL(info) << "targetDir member has been set in Workflow constructor.";
	}

	// If path is not a directory then log the error, print error message
	if (!(boost::filesystem::is_directory(input_dir)))
	{
		// Path received in arg[1] of cmd line entry is not a directory error
		BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: arg[1] is not a directory";
		exit(1);
	}
}
void Workflow::setTempDirectory(std::string s)
{
	// Create boost path object for intermediateDir containing intermediate files
	boost::filesystem::path intermediate_dir(s);

	// If path b is a directory, then assign the path to intermediateDir data member
	if (boost::filesystem::is_directory(intermediate_dir))
	{
		this->intermediate_dir_ = s;
		BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: intermediateDir member has been set in Workflow constructor.";
	}
	if (!(boost::filesystem::is_directory(intermediate_dir)))
	{
		// Path received in arg[2] of cmd line entry is not a directory error
		BOOST_LOG_TRIVIAL(warning) << "Warning in Workflow constructor: argv[2] is not a directory" << std::endl;
		BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: Creating directory at " << s << " now...";

		// Create a directory for the intermediate files to go 
		if (!(boost::filesystem::create_directory(s)))
		{
			// Directory for intermediate files failed to be created
			BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: directory for intermediate files was not created.";
			exit(1);
		}

		// Log that the directory was created 
		else
		{
			this->intermediate_dir_ = s;
			BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: Directory for intermediate files created.";
		}
	}

	// Else log that the directory was created 
	else
	{
		BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: Directory for intermediate files created.";
	}
}
void Workflow::setOutputDirectory(std::string s)
{
	// Create boost path object for outputDir that will contain output files of workflow object
	boost::filesystem::path output_dir(s);

	// If path c is a directory, then assign the path to outDir data member
	if (boost::filesystem::is_directory(output_dir))
	{
		this->out_dir_ = s;
		BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: outDir member has been set in Workflow constructor.";
	}
	if (!(boost::filesystem::is_directory(output_dir)))
	{
		// Path received in arg[3] of cmd line entry is not a directory error
		BOOST_LOG_TRIVIAL(warning) << "Warning in Workflow constructor: argv[3] is not a directory" << std::endl;
		BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: Creating directory at " << s << " now...";
		// Create a directory for the output files to go 
		if (!(boost::filesystem::create_directory(s)))
		{
			// std::cout << "Error creating directory: " << outputDir << std::endl;
			BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: directory for output files was not created.";
			exit(1);
		}
		// Log that the directory was created
		else
		{
			this->out_dir_ = s;
			BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: Directory for output files created.";
		}
	}
}

// Getter for target_dir_ data member
boost::filesystem::path Workflow::getTargetDir()
{
	// This function returns the boost::filesystem::path object private data member targetDir
	return this->target_dir_;
}
// Getter for intermediate_dir_ data member
boost::filesystem::path Workflow::getIntermediateDir()
{
	// This function returns the boost::filesystem::path object private data member intermediateDir
	return this->intermediate_dir_;
}
// Getter for out_dir_ data member
boost::filesystem::path Workflow::getOutDir()
{
	// This function returns the boost::filesystem::path object private data member outDir
	return this->out_dir_;
}

// Run a workflow consisting of map, sort and reduce on all files in target directory
void Workflow::run()
{

	boost::filesystem::ifstream input_stream;
	std::string key;
	std::string value;

	int success = 0;
	int line_count = 0;

	// Get all the input files in the input directory and verify they are valid
	std::vector<boost::filesystem::path> input_files;
	boost::filesystem::directory_iterator end_itr;

	for (boost::filesystem::directory_iterator itr(this->target_dir_); itr != end_itr; ++itr)
	{
		if (boost::filesystem::is_regular_file(itr->path()))
		{
			input_files.push_back(itr-> path());
		}
		else
		{
			// Log that a non regular file was encountered and ignored
			BOOST_LOG_TRIVIAL(info) << "Info in Workflow::run(): Non regular file: " << itr->path() << " detected and ignored.";
		}
	}

	int mapper_count = std::min<int>(static_cast<int>(input_files.size()), this->num_mappers_); // Not possible to have more mappers than input files
	int reducer_count = std::min<int>(static_cast<int>(input_files.size()), this->num_reducers_); // Not possible to have more reducers than input files
	std::vector<std::vector<boost::filesystem::path>> map_partitions = partitionFiles(input_files, mapper_count);


	// Sen map requests to the stubs
	MsgPassingCommunication::Message map_message;
	int stub = 0;
	for (int m = 0; m < map_partitions.size(); m++) {
		map_message = createMapMessage(this->stubs_[stub], map_partitions[m], reducer_count);
		this->controller_->postMessage(map_message);
		BOOST_LOG_TRIVIAL(info) << "Sent map request to " << this->stubs_[stub].toString();
		BOOST_LOG_TRIVIAL(debug) << "Message contents: " << map_message.toString();
		stub = (stub + 1) % this->stubs_.size();
	}

	// Wait until all the map processes finish
	int map_proc_complete = 0;
	MsgPassingCommunication::Message received_message;
	while (map_proc_complete != mapper_count) {
		received_message = this->controller_->getMessage();

		if (received_message.name().compare("heartbeat") == 0) {
			BOOST_LOG_TRIVIAL(info) << "Map process heartbeat: " << received_message.attributes()["message"];
		}

		// A map process finished
		if (received_message.name().compare("success") == 0) {
			BOOST_LOG_TRIVIAL(info) << "Map process completed: " << received_message.attributes()["message"];
			map_proc_complete++;
		}

		if (received_message.name().compare("failure") == 0) {
			// Map process failed
			BOOST_LOG_TRIVIAL(fatal) << "Map process failed. Error: " << received_message.attributes()["message"];
			exit(1);
		}

	}


	// Group together all the partition files from the mappers to pass to the reducer threads
	std::vector<std::vector<boost::filesystem::path>> reduce_partitions(this->num_reducers_);
	for (int p = 0; p < reduce_partitions.size(); p++) {
		for (boost::filesystem::directory_iterator itr(this->intermediate_dir_); itr != end_itr; ++itr)
		{
			if (itr->path().filename().string().find("p" + std::to_string(p)) != std::string::npos)
			{
				reduce_partitions[p].push_back(itr->path());
			}
		}
	}


	// Send reduce requests to the stubs
	MsgPassingCommunication::Message reduce_message;
	stub = 0;
	for (int r = 0; r < reduce_partitions.size(); r++) {
		reduce_message = createReduceMessage(this->stubs_[stub], reduce_partitions[r], this->intermediate_dir_, r);
		this->controller_->postMessage(reduce_message);
		stub = (stub + 1) % this->stubs_.size();
	}

	// Wait until all the reduce processes finish
	int reduce_proc_complete = 0;
	while (reduce_proc_complete != reducer_count) {
		received_message = this->controller_->getMessage();
	
		if (received_message.name().compare("heartbeat") == 0) {
			BOOST_LOG_TRIVIAL(info) << "Reduce process heartbeat: " << received_message.attributes()["message"];
		}

		// A map process finished
		if (received_message.name().compare("success") == 0) {
			BOOST_LOG_TRIVIAL(info) << "Reduce process completed: " << received_message.attributes()["message"];
			reduce_proc_complete++;
		}

		if (received_message.name().compare("failure") == 0) {
			// Map process failed
			BOOST_LOG_TRIVIAL(fatal) << "Reduce process failed. Error: " << received_message.attributes()["message"];
			exit(1);
		}
	}
	
	// Gather all the intermediate reduce files
	std::vector<boost::filesystem::path> reducer_output;
	for (boost::filesystem::directory_iterator itr(this->intermediate_dir_); itr != end_itr; ++itr)
	{
		if (itr->path().filename().string().find("reduce") != std::string::npos)
		{
			reducer_output.push_back(itr->path());
		}
	}
	
	// Run final reduce operation on intermediate reduce output files
	reduce_message = createReduceMessage(this->stubs_[0], reducer_output, this->out_dir_, 0);
	this->controller_->postMessage(reduce_message);

	// Wait until final reduce operation succeeds
	received_message = this->controller_->getMessage();
	while (received_message.name().compare("success") != 0) {
		received_message = this->controller_->getMessage();

		// Heartbeat messaage
		if (received_message.name().compare("heartbeat") == 0) {
			BOOST_LOG_TRIVIAL(info) << "Reduce process heartbeat: " << received_message.attributes()["message"];
		}

		// Reduce process failed
		if (received_message.name().compare("failure") == 0) {
			BOOST_LOG_TRIVIAL(fatal) << "Reduce process failed. Error: " << received_message.attributes()["message"];
			exit(1);
		}
	}


	// Final reduce operation succeeded
	BOOST_LOG_TRIVIAL(info) << "Reduce process success: " << received_message.attributes()["message"];

	// Write the success file
	BOOST_LOG_TRIVIAL(info) << "Writing success file...";
	boost::filesystem::path success_path(this->out_dir_.string() + "\\SUCCESS");
	boost::filesystem::ofstream success_file{ success_path };
	success_file.close();

	// Log success of workflow run
	BOOST_LOG_TRIVIAL(info) << "Map reduce process complete.";
}

// Partitions/groups a list of files/paths into the given number of partitions
// If there are less files than the number of partitions, partitions may be empty
std::vector<std::vector<boost::filesystem::path>> Workflow::partitionFiles(const std::vector<boost::filesystem::path>& files, 
	int partitions)
{
	std::vector<std::vector<boost::filesystem::path>> file_partitions(partitions);

	int partition = 0;
	// Place one file in a partition at a time
	// If there are less files than the number of partitions, some partitions will be empty
	for (int i = 0; i < files.size(); i++) {
		file_partitions[partition].push_back(files[i]);
		partition = (partition + 1) % partitions;
	}

	return file_partitions;
}
