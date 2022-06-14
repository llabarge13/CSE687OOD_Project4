// workflow.h
// Todd Hricik
// CSE687 Object Oriented Design
// April 28, 2022 - Project 1
// 
//  Workflow class definition
// 
// May 12, 2022 - Updated for project 2
//  Updated for reduce and map DLLs
#pragma once
#include "boost\filesystem.hpp"
#include "imap.h"
#include "sorting.h"
#include "ireduce.h"

typedef IMap<std::string, std::string>* (*buildMapper)(const boost::filesystem::path&);
typedef IReduce<std::string, int>* (*buildReducer)(const boost::filesystem::path&);

class Workflow
{
public:
	Workflow(std::string input_dir_arg,
		std::string inter_dir_arg,
		std::string output_dir_arg,
		std::string map_dll_path,
		std::string reduce_dll_path,
		int num_mappers,
		int num_reducers);														// constructor
	~Workflow();																// destructor
	boost::filesystem::path getTargetDir();										// directory containing files to be fed to Map
	boost::filesystem::path getIntermediateDir();								// directory containing intermediate files from Map
	boost::filesystem::path getOutDir();										// directory containing output file(s) from reduce
	boost::filesystem::path getMapLibPath();									// path to map dll
	boost::filesystem::path getReduceLibPath();									// path to reduce dll
	void run();																	// Runs the map sort reduce workflow


private:
	boost::filesystem::path target_dir_;
	boost::filesystem::path intermediate_dir_;
	boost::filesystem::path out_dir_;
	boost::filesystem::path map_lib_path_;
	boost::filesystem::path reduce_lib_path_;


	// Interfaces to map and reduce librarys 
	buildMapper create_map_;
	buildReducer create_reduce_;

	// Handles to map and reduce DLLs
	HINSTANCE hDLL_map_;
	HINSTANCE hDLL_reduce_;

	// Configurable number of map and reduce threads
	int num_mappers_;
	int num_reducers_;

	// Validators and setters
	void setInputDirectory(std::string input_dir_arg);
	void setTempDirectory(std::string temp_dir_arg);
	void setOutputDirectory(std::string output_dir_arg);

	// Aquire neccessary DLL handles for map and reduce libraryies
	void aquireMapDLL(std::string pathToMapDLL);
	void aquireReduceDLL(std::string pathToReduceDLL);

	// Partitions files into N partitions/groups
	static std::vector<std::vector<boost::filesystem::path>> partitionFiles(const std::vector<boost::filesystem::path>& files, int partitions);

	void runMapProcess(const std::vector<boost::filesystem::path>& files, int partitions);

	void runReduceProcess(const std::vector<boost::filesystem::path>& files, const boost::filesystem::path& output_directory, int partition);

};

