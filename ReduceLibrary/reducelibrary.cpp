// reduce.cpp
// Lyndsay LaBarge, Todd Hricik
// CSE687 Object Oriented Design
// Project 1
// April 28, 2022
// 
// Reduce class implementation
#include "reducelibrary.h"
#include "framework.h"

Reduce::Reduce(const boost::filesystem::path& directory) : IReduce<std::string, int>(directory)
{
}

// Performs reduce operation on intermediate results.
// Given sorted data from the intermediate file, reduces the results 
// by aggregating the values and output result to a file.
// e.g. (word, [1, 1]) -> (word, 2)
// If reduce operation fails (cannot write to file), returns -1. 
// If reduce operation succeeds, returns 0.
int Reduce::reduce(const std::string& key, const std::vector<int>& values)
{
	// Run reduce
	int result = std::reduce(std::execution::seq, values.begin(), values.end());
	// Write reduced value
	return exportToDisk(key, result);
}

REDUCELIBRARY_API Reduce* createReducer(const boost::filesystem::path& directory)
{
	return new Reduce(directory);
}

REDUCELIBRARY_API void destoryReducer(const Reduce* reducer)
{
	delete reducer;
}
