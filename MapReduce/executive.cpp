// executive.cpp
// Todd Hricik
// CSE687 Object Oriented Design
// April 28, 2022 - Project 1
// 
// Executive class implementation
// 
// May 12, 2022 - Updated for Project 2
//	Includes map and reduce DLL paths
// June 19, 2022 - Update for Project 4
//	Includes controller port and stub endpoints
//	Removed map and reduce DLL paths
#include <string>
#include "executive.h"
#include "workflow.h"


/*	Constructor takes input directory, dir for intermediate files, output directory,
	path to Map DLL and path to Reduce DLL, and number of mappers and reducers as args
*/
Executive::Executive(std::string input_dir_arg, 
	std::string inter_dir_arg, 
	std::string output_dir_arg, 
	int num_mappers,
	int num_reducers,
	int controller_port,
	std::vector<std::string> stub_endpoints)
{
	workflow_ = new Workflow(input_dir_arg, 
		inter_dir_arg, 
		output_dir_arg, 
		num_mappers,
		num_reducers,
		controller_port,
		stub_endpoints);
}

// Destructor
Executive::~Executive()
{
	delete workflow_;
}

// Calls on workflow object to run its map/sort/reduce application
void Executive::run()
{
	workflow_->run();
}