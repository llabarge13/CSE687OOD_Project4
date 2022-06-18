// sorting.cpp
// Lyndsay LaBarge
// CSE687 Object Oriented Design
// Project 1
// April 28, 2022
// 
// Sorting class implementation
#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "sorting.h"

// Sort map output from an input file.
// Input file must have the following format
//		(word, 1)
//		(word, 1)
//		...
// If the file cannot be parsed, returns -1.
// If the sort operation succeeds, retuns 0.
int Sorting::sort(const boost::filesystem::path& file)
{
	boost::filesystem::ifstream map_output(file);
	// File could not be opened
	if (map_output.fail()) {
		return -1;
	}

	std::string line;
	while (getline(map_output, line)) {
		// Parse out the key and the value
		boost::tokenizer<> tok(line);
		boost::tokenizer<>::iterator beg = tok.begin();
		std::string key = *beg;
		beg++;
		std::string value = *beg;

		try {
			// Add the value to the key's container
			aggregate_data_[key].push_back(std::stoi(value));
		}
		catch ([[maybe_unused]] const std::invalid_argument& ia) {
			// Value could not be converted to an integer
			// File has incorrect format
			return -1;
		}
	}
	map_output.close();

	// Success
	return 0;
}

// Get aggregate/sorted data.
// Aggregate data is map of where the key is a word
// and the values are word counts. 
// e.g. (word, [1, 1])
const boost::container::map<std::string,
	std::vector<int>>&Sorting::getAggregateData()
{
	return aggregate_data_;
}
