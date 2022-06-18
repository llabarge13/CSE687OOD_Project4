#include <boost\log\trivial.hpp>
#include "stub.h"
#include "sorting.h"
#include "Comm.h"
#include "Sockets.h"
#include "Message.h"

Stub::Stub(std::string endpoint,
	const buildMapper& create_mapper,
	const buildReducer& create_reducer)
{

	// Parse the endpoint
	std::string stub_host;
	int stub_port;
	size_t pos = endpoint.find(':');
	if (pos != std::string::npos) {
		stub_host = endpoint.substr(0, pos);
		// Convert port to an integer
		try {
			stub_port = std::stoi(endpoint.substr(pos + 1));

		}
		catch (const std::exception&) {
			// Port is not an integer
			BOOST_LOG_TRIVIAL(fatal) << "Invalid stub endpoint " << endpoint;
			exit(1);
		}
		endpoint_ = new MsgPassingCommunication::EndPoint(stub_host, stub_port);

	}
	else {
		BOOST_LOG_TRIVIAL(fatal) << "Invalid stub endpoint " << endpoint;
		exit(1);
	}

	// Start the stub
	try {
		comm_ = new MsgPassingCommunication::Comm(*endpoint_, endpoint_->toString());
		comm_->start();
		BOOST_LOG_TRIVIAL(info) << "Started stub at " << endpoint_->toString();
	}
	catch (const std::exception&) {
		// Failed to start stub
		BOOST_LOG_TRIVIAL(fatal) << "Unable to start stub at " << endpoint_->toString();
		exit(1);
	}


	// Save the mappers
	create_map_ = create_mapper;
	create_reduce_ = create_reducer;
}

Stub::~Stub()
{
	delete endpoint_;
	comm_->stop();
}

const MsgPassingCommunication::EndPoint& Stub::getEndpoint() const
{
	return (*endpoint_);
}


void Stub::run()
{
	BOOST_LOG_TRIVIAL(info) << endpoint_->toString() << " started running";
	MsgPassingCommunication::Message msg;
	while (true) {
		BOOST_LOG_TRIVIAL(info) << endpoint_->toString() << " waiting for message.";
		msg = comm_->getMessage();
		BOOST_LOG_TRIVIAL(info) << endpoint_->toString() << "received messsage: " << msg.toString();
	
		std::unordered_map<std::string, std::string> attrs = msg.attributes();
		// Map request
		if (msg.command().compare("run_map") 
			&& msg.containsKey("output_directory") 
			&& msg.containsKey("input_files")
			&& msg.containsKey("partitions")) {
			BOOST_LOG_TRIVIAL(info) << endpoint_->toString() << "received map request.";
			BOOST_LOG_TRIVIAL(info) << "map - input files:" << attrs["input_files"];
			BOOST_LOG_TRIVIAL(info) << "map - output directory:" << attrs["output_directory"];
			BOOST_LOG_TRIVIAL(info) << "map - number of partitions:" << attrs["partitions"];


			// validate they are correct, if not send error message to controller
			boost::filesystem::path ouput_dir {attrs["output_directory"]};
			int partitions = std::stoi(attrs["partitions"]);
			// parse input files
		}
		

	}
}



void Stub::runMapProcess(const std::vector<boost::filesystem::path>& files, 
	const boost::filesystem::path& output_directory, 
	int num_partitions)
{
	// Instantiate a Map object via the IMap interface
	IMap<std::string, std::string>* mapper = create_map_(output_directory);

	// Partition the input files
	std::vector<std::vector<boost::filesystem::path>> partitions = partitionFiles(files, num_partitions);

	// Get the id of the thread to use in the partition file names
	std::stringstream ss;
	ss << std::this_thread::get_id();
	std::string thread_id = ss.str();

	// Process the files in each partition with map
	boost::filesystem::path input_file_path;
	boost::filesystem::ifstream input_stream;
	std::string line;
	for (int partition = 0; partition < partitions.size(); partition++) {
		// Output file name is thread id + p{parition number}
		std::string partition_name = thread_id + "p" + std::to_string(partition);

		for (int file = 0; file < partitions[partition].size(); file++) {
			// Open the input file
			boost::filesystem::path current_file = partitions[partition][file];
			input_stream.open(current_file);

			// File could not be opened
			if (input_stream.fail() == 1)
			{
				// TODO: change to error message back to controller
				BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow run(): ifstream could not be opened for " << current_file.filename().string();
				exit(1);
			}

			int success = 0;
			int line_count = 0;
			BOOST_LOG_TRIVIAL(info) << "Info in Workflow run(): Running map process for " << current_file.filename().string();
			// Process all lines of the file via map
			while (std::getline(input_stream, line))
			{
				// Increment the line count and call the map object to map the key and the value
				line_count++;
				success = mapper->map(partition_name, line);

				// If the map member function of the map object does not return zero (which is a success), then log
				if (success != 0)
				{
					// TODO: Change to erroer message back to controller
					BOOST_LOG_TRIVIAL(error) << "Error in Workflow run(): Map did not successfully process entire file at line " << line_count << " of " << current_file.filename().string();
					exit(1);
				}
			}

			// Log map process has completed for the current file
			BOOST_LOG_TRIVIAL(info) << "Map process completed for " << current_file.filename().string();

			// Close the current input stream
			input_stream.close();
			line_count = 0;
		}
	}

	delete mapper;
}

// Takes as input all the files that belong to a particular partition along with the partition id (e.g 0) and the directory to output the reduce file to.
// Because we are running multiple reducers, we will get multiple reduce files at the end when the threads return.
// We will have to run one final reduce process over all those intermediate files e.g. reduce0.txt, reduce1.txt if we had 2 partitions
// The intermeidate reduce files should go in the intermediate dir, not the final output directory, which is why there is an output directory parameter.
void Stub::runReduceProcess(const std::vector<boost::filesystem::path>& files, 
	const boost::filesystem::path& output_directory, 
	int partition)
{
	// Create reducer
	// The intermeidate reduce files should go in the intermediate dir, not the final output directory, which is why there is an output directory parameter.
	IReduce<std::string, int>* reducer = create_reduce_(output_directory);
	// Set the output file name to reduce + partition e.g. reduce0.txt
	std::string output_filename = "reduce" + std::to_string(partition) + ".txt";
	reducer->setOutputFileName(output_filename);

	// Run sort on all the files that belong to the partition
	int sort_success = 0;
	Sorting* sorter = new Sorting();

	for (int file = 0; file < files.size(); file++)
	{
		BOOST_LOG_TRIVIAL(info) << "Running sort on " << files[file].filename().string();
		sort_success = sorter->sort(files[file]);

		if (sort_success != 0) {
			// TODO: Change to message to controller
			BOOST_LOG_TRIVIAL(fatal) << "Failed to process " << files[file].filename().string() << " with sort.";
			exit(1);
		}
	}

	// Run reduce on the output from sort
	BOOST_LOG_TRIVIAL(info) << "Running reduce operation...";
	int reducer_success = 0;
	for (auto const& pair : sorter->getAggregateData())
	{
		reducer_success = reducer->reduce(pair.first, pair.second);
		// TODO: Change to message to controller
		if (reducer_success != 0) {
			BOOST_LOG_TRIVIAL(fatal) << "Failed to export to " << reducer->getOutputPath().string() << " with reduce.";
			exit(1);
		}
	}
	// Delete reducer
	delete sorter;
	delete reducer;
}

// Partitions/groups a list of files/paths into the given number of partitions
// If there are less files than the number of partitions, partitions may be empty
std::vector<std::vector<boost::filesystem::path>> Stub::partitionFiles(const std::vector<boost::filesystem::path>& files,
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

