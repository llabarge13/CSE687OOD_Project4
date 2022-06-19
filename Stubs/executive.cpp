// executive.cpp
// Lyndsay LaBarge, Todd Hricik
// CSE687 Object Oriented Design
// 
// June 19, 2022 - Project 4
// 
// StubExecutive class implementation.
// StubExecutive class loads map and reduce DLLs 
// and starts stub threads at the given endpoints
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <regex>
#include <algorithm> 
#include <boost\filesystem.hpp>
#include <boost\filesystem\fstream.hpp>
#include "stub.h"
#include "executive.h"

// Construct a new stub excutive
StubExecutive::StubExecutive(std::string map_dll_file,
	std::string reduce_dll_file,
	std::vector<std::string> stub_endpoints) {
	
	// Load DLLs
	acquireMapDLL(map_dll_file);
	acquireReduceDLL(reduce_dll_file);

	// Start stub threads
	createStubs(stub_endpoints);
}



StubExecutive::~StubExecutive() {

	// Free DLLs
	FreeLibrary(hDLL_map_);
	FreeLibrary(hDLL_reduce_);

}

// Acquire functions in the MapLibrary DLL
void StubExecutive::acquireMapDLL(std::string path_to_dll)
{

	// Check map DLL is a regular file 
	if (!(boost::filesystem::is_regular_file(path_to_dll)))
	{
		BOOST_LOG_TRIVIAL(fatal) << "Fatal in Executive constructor: Map DLL is not a regular file.";
		exit(1);
	}

	// Else get DLL handle for map
	else
	{
		std::wstring widestr = std::wstring(path_to_dll.begin(), path_to_dll.end());
		const wchar_t* widecstr = widestr.c_str();

		// Create a handle to map DLL
		hDLL_map_ = LoadLibraryEx(widecstr, NULL, NULL);   // Handle to map DLL
		if (hDLL_map_ != NULL) {
			BOOST_LOG_TRIVIAL(info) << "Info in Executive constructor: Map DLL located.";
			create_map_ = (buildMapper)GetProcAddress(hDLL_map_, "createMapper");

			// If function pointer to createMap fails to be created, log and exit
			if (create_map_ == NULL)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Fatal in Executive constructor: Function pointer to createMap is NULL.";
				exit(1);
			}
			this->map_lib_path_ = boost::filesystem::path{ path_to_dll };
		}

		// Else log that handle of Map DLL failed to be created and exit
		else
		{
			BOOST_LOG_TRIVIAL(fatal) << "Fatal in Executive constructor: Failed to get handle of map DLL.";
			exit(1);
		}

	}
}

// Acquire functions in the ReduceLibrary DLL
void StubExecutive::acquireReduceDLL(std::string path_to_dll)
{
	// Check reduce DLL is a regular file 
	if (!(boost::filesystem::is_regular_file(path_to_dll)))
	{
		BOOST_LOG_TRIVIAL(fatal) << "Fatal in Executive constructor: Reduce DLL is not a regular file.";
		exit(1);
	}
	// Else attempt to get a handle for the Reduce DLL
	else
	{
		std::wstring widestr = std::wstring(path_to_dll.begin(), path_to_dll.end());
		const wchar_t* widecstr = widestr.c_str();

		hDLL_reduce_ = LoadLibraryEx(widecstr, NULL, NULL);   // Handle to Reduce DLL
		if (hDLL_reduce_ != NULL) {
			BOOST_LOG_TRIVIAL(info) << "Info in Executive constructor: Reduce DLL located.";
			create_reduce_ = (buildReducer)GetProcAddress(hDLL_reduce_, "createReducer");

			// If create_reduce_ function pointer is NULL, then log and exit
			if (create_reduce_ == NULL)
			{
				BOOST_LOG_TRIVIAL(fatal) << "Fatal in Executive constructor: Function pointer to create_reduce_ is NULL.";
				exit(1);
			}
			this->reduce_lib_path_ = boost::filesystem::path{ path_to_dll };
		}
		// Else log failure to get Reduce DLL handle and exit
		else
		{
			BOOST_LOG_TRIVIAL(fatal) << "Fatal in Executive constructor: Failed to get handle of reduce DLL.";
			exit(1);
		}
	}

}


// Create a new stub at each endpoints
void StubExecutive::createStubs(std::vector<std::string> stub_endpoints) {
	std::vector<std::thread> stub_threads;
	for (const std::string& ep : stub_endpoints) {
		stub_threads.push_back(std::thread(StubExecutive::stubProc, ep, create_map_, create_reduce_));
	}
	
	for (auto t = stub_threads.begin(); t != stub_threads.end(); t++) {
		t->detach();
	}
}

void StubExecutive::stubProc(std::string endpoint, const buildMapper& map, const buildReducer& reduce)
{
	Stub stub(endpoint, map, reduce);
	stub.run();
}
