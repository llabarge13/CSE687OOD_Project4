// executive.h
// Todd Hricik
// CSE687 Object Oriented Design
// April 28, 2022 - Project 1
// 
//  Executive class definition
//
// May 12, 2022 - Updated for Project 2
//	Now includes paths for reduce and map DLLs
//
// June 2, 2022 - Update for Project 3
//	Now includes configurable number of map threads and reduce threads
#include <string>
#include "workflow.h"
#pragma once

// Executive takes command line arguments into constructor
// and instantiates a Workflow object that will run the 
// map reduce application.
class Executive
{
	public:
		Executive(std::string target_dir, 
			std::string intermediate_dir,
			std::string out_dir,
			int num_mappers,
			int num_reducers,
			int controller_port,
			std::vector<std::string> stub_endpoints);

		~Executive();
		// Calls workflow to run
		void run();

	private:
		Workflow* workflow_;
};
