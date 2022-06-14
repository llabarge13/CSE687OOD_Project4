// ireduce.h
// Lyndsay LaBarge, Todd Hricik
// CSE687 Object Oriented Design
// May 12, 2022
// 
// Template class IReduce is a base clase that serves as an interface
// for the reduce library.
// 
// Abstract class.
//
// June 2, 2022 - Project 3
//	Added class method to change output file name
#include <array>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#pragma once


template <typename KEYT, typename VALUET>
class IReduce
{

public:

	// Create a new IReduce object that will write results to
	// the given directory
	IReduce(const boost::filesystem::path& directory);

	// IReduce concrete object must be deleted or go out of scope
	// and be destroyed on the stack, otherwise the output
	// file will not be able to be read until the object
	// class relinquishes it .
	~IReduce();

	// Performs reduce operation on intermediate results.
	// If reduce operation fails (cannot write to file), returns -1. 
	// If reduce operation succeeds, returns 0.
	//
	// Abstract class method, must be defined.
	virtual int reduce(const KEYT& key, const std::vector<VALUET>& values) = 0;

	// Gets the path of the output file
	boost::filesystem::path getOutputPath();

	// Gets the path of the output directory
	boost::filesystem::path getOutputDirectory();

	void setOutputFileName(std::string name);

private:
	boost::filesystem::path output_directory_;
	boost::filesystem::path output_path_;
	boost::filesystem::ofstream* output_stream_;
	std::string output_file_name_ = "reduce.txt";

protected:
	// Exports word, value pair to disk
	// If the export fails, returns -1. 
	// If the export succeeds, returns 0.
	virtual int exportToDisk(const KEYT& key, VALUET value);
};

// IReduce Constructor
template<typename KEYT, typename VALUET>
inline IReduce<KEYT, VALUET>::IReduce(const boost::filesystem::path& directory)
{
	output_directory_ = directory;
	output_path_ = boost::filesystem::path{
		output_directory_.string() + "\\" + output_file_name_
	};
	output_stream_ = new boost::filesystem::ofstream{ };

	// Delete old result file if it exists
	if (boost::filesystem::exists(output_path_)) {
		boost::filesystem::remove(output_path_);
	}
}

// Destructor
template<typename KEYT, typename VALUET>
inline IReduce<KEYT, VALUET>::~IReduce()
{
	// Flush & close the file stream
	if (output_stream_->is_open()) {
		output_stream_->close();
	}
	delete output_stream_;
}

// Getter for output_path_ data member
template<typename KEYT, typename VALUET>
inline boost::filesystem::path IReduce<KEYT, VALUET>::getOutputPath()
{
	return output_path_;
}

// Getter for output_directory_ data member
template<typename KEYT, typename VALUET>
inline boost::filesystem::path IReduce<KEYT, VALUET>::getOutputDirectory()
{
	return output_directory_;
}

template<typename KEYT, typename VALUET>
inline void IReduce<KEYT, VALUET>::setOutputFileName(std::string name)
{
	// Update the output path
	output_file_name_ = name;
	output_path_ = boost::filesystem::path{
		output_directory_.string() + "\\" + output_file_name_
	};

	// Close old output stream if it is open
	if (output_stream_->is_open()) {
		output_stream_->close();
	}
	output_stream_ = new boost::filesystem::ofstream{ };


	// Delete old result file if it exists
	if (boost::filesystem::exists(output_path_)) {
		boost::filesystem::remove(output_path_);
	}
}

// Export to disk writes (key, sum values) to output_file_
template<typename KEYT, typename VALUET>
inline int IReduce<KEYT, VALUET>::exportToDisk(const KEYT& key, VALUET value)
{
	// Open the output_stream_ if it is not open
	if (!output_stream_->is_open()) {
		output_stream_->open(output_path_, std::ios_base::app);
	}

	// If output_stream_ does not fail to open, then output (key, reduced value) to output stream
	if (!output_stream_->fail()) {
		// Write result to disk 
		*(output_stream_) << "(";
		*(output_stream_) << key;
		*(output_stream_) << ", ";
		*(output_stream_) << value;
		*(output_stream_) << ")\n";
		output_stream_->close();
		return 0;
	}
	// Else return -1 to flag a failure
	else {
		return -1;
	}
}
