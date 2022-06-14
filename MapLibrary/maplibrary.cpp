// maplibrary.cpp
// Lyndsay LaBarge, Todd Hricik
// CSE687 Object Oriented Design
// May 12, 2022
// 
// Map class and library definition.
#include "maplibrary.h"
#include "framework.h"

// Map constructor takes boost filesystem path to a directory as input
Map::Map(const boost::filesystem::path& directory) : IMap<std::string, std::string>(directory)
{
}

// Given data from a file, writes to a separate temporary file that 
// holds (word, 1) for each occurrence of every word.
// If the map operation failed (could not write to the file), 
// returns -1.
// If the map operation succeeded, returns 0.
int Map::map(const std::string& key, const std::string& value)
{
	int export_success;
	boost::tokenizer<> tok(value);
	// Tokenize text
	for (boost::tokenizer<>::iterator beg = tok.begin();
		beg != tok.end();
		++beg) {
		// Normalize text (remove capitalization)
		std::string token = *beg;
		boost::algorithm::to_lower(token);
		// Export token
		export_success = exportToDisk(key, token, 1);

		// Export failed
		if (export_success != 0) {
			return -1;
		}
	}
	// Map success
	return 0;
}

// Function pointer to Map constructor
MAPLIBRARY_API Map* createMapper(const boost::filesystem::path& directory)
{
	return new Map(directory);
}

// destroyMapper deletes pointer to concrete Map object.
MAPLIBRARY_API void destoryMapper(const Map* mapper)
{
	delete mapper;
}