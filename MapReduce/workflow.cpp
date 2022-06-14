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
#include "sorting.h"
#include "workflow.h"
#include "windows.h"

// Constructor that creates boost::filesystem::path objects for the input directory, intermediate files directory and the output directory
Workflow::Workflow(std::string input_dir_arg,
	std::string inter_dir_arg,
	std::string output_dir_arg,
	std::string map_dll_path,
	std::string reduce_dll_path,
	int num_mappers,
	int num_reducers)
{
	BOOST_LOG_TRIVIAL(debug) << "Debug in Workflow constructor: Entering constructor.";

	this->num_mappers_ = num_mappers;
	this->num_reducers_ = num_reducers;

	// Get DLL handle for map library
	aquireMapDLL(map_dll_path);

	// Get DLL handlwe for reduce library
	aquireReduceDLL(reduce_dll_path);

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

	FreeLibrary(hDLL_map_);
	FreeLibrary(hDLL_reduce_);
}

// Aquire functions in the MapLibrary DLL
void Workflow::aquireMapDLL(std::string path_to_dll)
{

	// Check map DLL is a regular file 
	if (!(boost::filesystem::is_regular_file(path_to_dll)))
	{
		BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: Map DLL is not a regular file.";
		exit(-1);
	}

	// Else get DLL handle for map
	else
	{
		std::wstring widestr = std::wstring(path_to_dll.begin(), path_to_dll.end());
		const wchar_t* widecstr = widestr.c_str();

		// Create a handle to map DLL
		hDLL_map_ = LoadLibraryEx(widecstr, NULL, NULL);   // Handle to map DLL
		if (hDLL_map_ != NULL) {
			BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: Map DLL located.";
			create_map_ = (buildMapper)GetProcAddress(hDLL_map_, "createMapper");

			// If function pointer to createMap fails to be created, log and exit
			if (create_map_ == NULL)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: Function pointer to createMap is NULL.";
				exit(-1);
			}
			this->map_lib_path_ = boost::filesystem::path{ path_to_dll };
		}

		// Else log that handle of Map DLL failed to be created and exit
		else
		{
			BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: Failed to get handle of map DLL.";
			exit(-1);
		}

	}
}

// Aquire functions in the ReduceLibrary DLL
void Workflow::aquireReduceDLL(std::string path_to_dll)
{
	// Check reduce DLL is a regular file 
	if (!(boost::filesystem::is_regular_file(path_to_dll)))
	{
		BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: Reduce DLL is not a regular file.";
		exit(-1);
	}
	// Else attempt to get a handle for the Reduce DLL
	else
	{
		std::wstring widestr = std::wstring(path_to_dll.begin(), path_to_dll.end());
		const wchar_t* widecstr = widestr.c_str();

		hDLL_reduce_ = LoadLibraryEx(widecstr, NULL, NULL);   // Handle to Reduce DLL
		if (hDLL_reduce_ != NULL) {
			BOOST_LOG_TRIVIAL(info) << "Info in Workflow constructor: Reduce DLL located.";
			create_reduce_ = (buildReducer)GetProcAddress(hDLL_reduce_, "createReducer");

			// If create_reduce_ function pointer is NULL, then log and exit
			if (create_reduce_ == NULL)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: Function pointer to create_reduce_ is NULL.";
				exit(-1);
			}
			this->reduce_lib_path_ = boost::filesystem::path{ path_to_dll };
		}
		// Else log failure to get Reduce DLL handle and exit
		else
		{
			BOOST_LOG_TRIVIAL(fatal) << "Fatal in Workflow constructor: Failed to get handle of reduce DLL.";
			exit(-1);
		}
	}

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
		exit(-1);
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
			exit(-1);
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
			exit(-1);
		}
		// Log that the directory was 
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

// Getter for map dll path
boost::filesystem::path Workflow::getMapLibPath()
{
	return this->map_lib_path_;
}

// Getter for reduce dll path
boost::filesystem::path Workflow::getReduceLibPath()
{
	return this->reduce_lib_path_;
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

	// TODO: make number of mappers and reducers configurable via the command line
	int mapper_count = std::min<int>(static_cast<int>(input_files.size()), this->num_mappers_); // Not possible to have more mappers than input files
	int reducer_count = std::min<int>(static_cast<int>(input_files.size()), this->num_reducers_); // Not possible to have more reduces than input files
	std::vector<std::vector<boost::filesystem::path>> map_partitions = partitionFiles(input_files, mapper_count);


	// Create the mapper threads
	std::vector<std::thread> map_threads;
	for (int m = 0; m < map_partitions.size(); m++) {
		map_threads.push_back(std::thread(&Workflow::runMapProcess, this, map_partitions[m], reducer_count));
	}

	// Wait for all the map threads to finish
	for (auto t = map_threads.begin(); t != map_threads.end(); t++) {
		t->join();
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

	// Create the reduce threads
	std::vector<std::thread> reduce_threads;
	for (int r = 0; r < reduce_partitions.size(); r++) {
		reduce_threads.push_back(std::thread(&Workflow::runReduceProcess, this, reduce_partitions[r], this->intermediate_dir_, r));
	}

	// Wait for all the reduce threads to finish
	for (auto t = reduce_threads.begin(); t != reduce_threads.end(); t++) {
		t->join();
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
	runReduceProcess(reducer_output, this->out_dir_, 0);


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

void Workflow::runMapProcess(const std::vector<boost::filesystem::path>& files, int num_partitions)
{
	// Instantiate a Map object via the IMap interface
	IMap<std::string, std::string>* mapper = create_map_(this->intermediate_dir_);

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
					BOOST_LOG_TRIVIAL(error) << "Error in Workflow run(): Map did not successfully process entire file at line " << line_count << " of " << current_file.filename().string();
					exit(1);
				}
			}

			// Log map process has completed for the current file
			BOOST_LOG_TRIVIAL(info) << "Info in Workflow run(): Map process completed for " << current_file.filename().string();

			// Close the current input stream
			input_stream.close();
			line_count = 0;
		}
	}

	delete mapper;
}

	// Workflow::runReduceProcess takes as input all the files that belong to a particular partition
	// along with the partition id (e.g 0) and the directory to output the reduce file to.
	// Because we are running multiple reducers, we will get multiple reduce files at the end when the threads return.
	// We will have to run one final reduce process over all those intermediate files e.g. reduce0.txt, reduce1.txt if we had 2 partitions
	// The intermeidate reduce files should go in the intermediate dir, not the final output directory, which is why there is an output directory parameter.
	void Workflow::runReduceProcess(const std::vector<boost::filesystem::path>&files, const boost::filesystem::path & output_directory, int partition)
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
			if (reducer_success != 0) {
				BOOST_LOG_TRIVIAL(fatal) << "Failed to export to " << reducer->getOutputPath().string() << " with reduce.";
				exit(1);
			}
		}
		// Delete reducer
		delete sorter;
		delete reducer;
	}