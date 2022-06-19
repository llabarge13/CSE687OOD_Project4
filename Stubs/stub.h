// stub.h
// Lyndsay LaBarge, Todd Hricik
// CSE687 Object Oriented Design
// 
// June 19, 2022 - Project 4
// 
// Stub class declaration
// A stub is a socket capable of running map or reduce processes.
#pragma once
#include <boost/filesystem.hpp>
#include <future>
#include "imap.h"
#include "ireduce.h"
#include "Comm.h"
#include "Sockets.h"
#include "Message.h"

typedef IMap<std::string, std::string>* (*buildMapper)(const boost::filesystem::path&);
typedef IReduce<std::string, int>* (*buildReducer)(const boost::filesystem::path&);

class Stub {

public:
	Stub(std::string endpoint, 
	const buildMapper& create_mapper,
	const buildReducer& create_reducer);
	~Stub();

	void run(); // start listening for messages
	void stop(); // shutdown the socket
	const MsgPassingCommunication::EndPoint& getEndpoint() const; // stub endpoint


private:
	// Stub socket
	MsgPassingCommunication::EndPoint endpoint_;
	MsgPassingCommunication::Comm* comm_;

	buildMapper create_map_;
	buildReducer create_reduce_;

	// Partitions files into N partitions/groups
	static std::vector<std::vector<boost::filesystem::path>> partitionFiles(const std::vector<boost::filesystem::path>& files, int partitions);

	// Parses an endpoint string e.g. localhost:9090 into an endpoint object
	static MsgPassingCommunication::EndPoint parseEndpoint(std::string);

	// Parses a comma seperated list of file paths
	static std::vector<boost::filesystem::path> parseFileList(std::string files);

	// Runs a map process
	void runMapProcess(const std::vector<boost::filesystem::path>& files, 
		const boost::filesystem::path& output_directory, 
		int partitions,
		MsgPassingCommunication::EndPoint client_endpoint);

	// Runs a reduce process
	void runReduceProcess(const std::vector<boost::filesystem::path>& files,
		const boost::filesystem::path& output_directory,
		int partition,
		MsgPassingCommunication::EndPoint client_endpoint);

	// A heartbeat thread that sends a heartbeat message to an endpoint
	void heartbeatThread(MsgPassingCommunication::EndPoint client_endpoint, int interval, std::string message, std::future<void> future);

	// Creates a heartbeat message
	MsgPassingCommunication::Message createHeartbeatMessage(MsgPassingCommunication::EndPoint client_endpoint, std::string message);

	// Creates a success message
	MsgPassingCommunication::Message createSuccessMessage(MsgPassingCommunication::EndPoint client_endpoint, std::string message);

	// Creates a failture messaage
	MsgPassingCommunication::Message createFailureMessage(MsgPassingCommunication::EndPoint client_endpoint, std::string message);
};