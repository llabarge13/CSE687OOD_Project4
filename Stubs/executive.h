// executive.h
// Lyndsay LaBarge, Todd Hricik
// CSE687 Object Oriented Design
// 
// June 19, 2022 - Project 4
// 
// StubExecutive class declaration.
// StubExecutive class loads map and reduce DLLs 
// and starts stub threads at the given endpoints
#pragma once
#include <boost\filesystem.hpp>
#include <boost\log\trivial.hpp>
#include "imap.h"
#include "ireduce.h"
#include "stub.h"
#include "Comm.h"
#include "Sockets.h"
#include "Message.h"

typedef IMap<std::string, std::string>* (*buildMapper)(const boost::filesystem::path&);
typedef IReduce<std::string, int>* (*buildReducer)(const boost::filesystem::path&);

class StubExecutive
{
public:
	StubExecutive(std::string map_dll_file, 
		std::string reduce_dll_file,
		std::vector<std::string> stub_endpoints);
	~StubExecutive();

private:
	Sockets::SocketSystem ss_;

	// Paths to map and reduce DLLs
	boost::filesystem::path map_lib_path_;
	boost::filesystem::path reduce_lib_path_;

	// Interfaces to map and reduce libraries
	buildMapper create_map_;
	buildReducer create_reduce_;

	// Handles to map and reduce DLLs
	HINSTANCE hDLL_map_;
	HINSTANCE hDLL_reduce_;

	// Acquire neccessary DLL handles for map and reduce libraries
	void acquireMapDLL(std::string pathToMapDLL);
	void acquireReduceDLL(std::string pathToReduceDLL);

	// Create the stubs
	void createStubs(std::vector<std::string> stub_endpoints);

	// Create a new stub and run it
	static void stubProc(std::string endpoint, const buildMapper& map, const buildReducer& reduce);
};