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
	const buildReducer& create_reducer); // needs dlls
	~Stub();

	void run();
	void stop();
	const MsgPassingCommunication::EndPoint& getEndpoint() const;


private:
	MsgPassingCommunication::EndPoint endpoint_;
	MsgPassingCommunication::Comm* comm_;

	buildMapper create_map_;
	buildReducer create_reduce_;

	// Partitions files into N partitions/groups
	static std::vector<std::vector<boost::filesystem::path>> partitionFiles(const std::vector<boost::filesystem::path>& files, int partitions);
	static MsgPassingCommunication::EndPoint parseEndpoint(std::string);
	static std::vector<boost::filesystem::path> parseFileList(std::string files);


	void runMapProcess(const std::vector<boost::filesystem::path>& files, 
		const boost::filesystem::path& output_directory, 
		int partitions,
		MsgPassingCommunication::EndPoint client_endpoint);

	void runReduceProcess(const std::vector<boost::filesystem::path>& files,
		const boost::filesystem::path& output_directory,
		int partition,
		MsgPassingCommunication::EndPoint client_endpoint);

	void heartbeatThread(MsgPassingCommunication::EndPoint client_endpoint, int interval, std::string message, std::future<void> future);
	MsgPassingCommunication::Message createHeartbeatMessage(MsgPassingCommunication::EndPoint client_endpoint, std::string message);
	MsgPassingCommunication::Message createSuccessMessage(MsgPassingCommunication::EndPoint client_endpoint, std::string message);
	MsgPassingCommunication::Message createFailureMessage(MsgPassingCommunication::EndPoint client_endpoint, std::string message);
};