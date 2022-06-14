// test.cpp
// Todd Hricik, Lyndsay LaBarge
// CSE687 Object Oriented Design
// 
// April 28, 2022 - Project 1
// 
// Google unit tests
//
// May 12, 2022
//  Updated tests for workflow to include loading
//  Map and Reduce DLLs
#include "pch.h"
#include <string>
#include <array>
#include <boost/filesystem.hpp>
#include <boost/container/map.hpp>
#include "../MapReduce/sorting.h"
#include "../MapLibrary/maplibrary.h"
#include "../ReduceLibrary/reducelibrary.h"
#include "../MapReduce/workflow.h"

// Verify map writes to the correct file
TEST(mapTest, checkOutputPath) {
    // Create the directory to write to if it does not already exist
    boost::filesystem::path directory = boost::filesystem::path{ ".\\temp" };
    if (!boost::filesystem::exists(boost::filesystem::status(directory))) {
        boost::filesystem::create_directory(directory);
    }

    // Run map
    std::string text = "The quick brown fox jumps over the lazy dog.";
    std::string file = "test.txt";

    Map* m = createMapper(directory);

    int success = m->map(file, text);
    std::cout << m->getOutputPath() << std::endl;
    EXPECT_EQ(success, 0);

    // Check the output path is the expected output path
    boost::filesystem::path output_path{ directory.string() + "\\" + file };
    EXPECT_EQ(output_path.compare(m->getOutputPath()), 0);
}

// Verify the data map wrote to the file is correct
TEST(mapTest, checkMapResult) {
    // Create the directory to write to if it does not already exist
    boost::filesystem::path directory = boost::filesystem::path{ ".\\temp" };
    if (!boost::filesystem::exists(boost::filesystem::status(directory))) {
        boost::filesystem::create_directory(directory);
    }

    // Run map
    std::string text = "The quick brown fox jumps over the lazy dog.";
    std::string file = "test.txt";
    Map* m = createMapper(directory);
    int success = m->map(file, text);
    EXPECT_EQ(success, 0);

    std::array<std::string, 9> map_result = {
        "(the, 1)",
        "(quick, 1)",
        "(brown, 1)",
        "(fox, 1)",
        "(jumps, 1)",
        "(over, 1)",
        "(the, 1)",
        "(lazy, 1)",
        "(dog, 1)"
    };
    boost::filesystem::ifstream map_output(m->getOutputPath());
    delete m;

    // Verify the output in the file is correct
    std::string line;
    for (int i = 0; i < map_result.size(); i++) {
        getline(map_output, line);
        EXPECT_EQ(line.compare(map_result[i]), 0);
    };

    map_output.close();
}

// Check map fails when cannot write to output file
TEST(mapTest, badOutputDirectory) {
    // Non-existent directory
    boost::filesystem::path directory = boost::filesystem::path{ ".\\tem" };
   
    // Run map
    std::string text = "The quick brown fox jumps over the lazy dog.";
    std::string file = "test.txt";
    Map* m = createMapper(directory);
    int success = m->map(file, text);

    EXPECT_EQ(success, -1);
}

// Verify sort creates the right output
TEST(sortTest, checkSortOutput) {
    boost::filesystem::path input_path = boost::filesystem::path{ ".\\temp\\test.txt" };
    
    // Sort the data in a map output file
    Sorting sorter = Sorting();
    int success = sorter.sort(input_path);
    EXPECT_EQ(success, 0);
    const boost::container::map<std::string, std::vector<int>>& aggregate_data = sorter.getAggregateData();

    // Expected output of sort
    boost::container::map<std::string, std::vector<int>> expected_data;
    std::vector<int> one{ 1 };
    std::vector<int> two{ 1, 1 };
    expected_data["the"] = two;
    expected_data["quick"] = one;
    expected_data["brown"] = one;
    expected_data["fox"] = one;
    expected_data["jumps"] = one;
    expected_data["over"] = one;
    expected_data["lazy"] = one;
    expected_data["dog"] = one;

    // Iterate over the output of sort and compare to expected output
    for (auto const& pair : aggregate_data) {
        for (int i = 0; i < pair.second.size(); i++) {
            EXPECT_EQ(aggregate_data.at(pair.first).at(i), expected_data.at(pair.first).at(i));
        }
    }
}

// Verify sort fails when cannot open input file
TEST(sortTest, badInputFilePath) {
    // Non-existent file
    boost::filesystem::path output_path = boost::filesystem::path{ ".\\temp\\tes.txt" };

    // Sort the data in a map output file
    Sorting sorter = Sorting();
    int success = sorter.sort(output_path);
    EXPECT_EQ(success, -1);
}

// Verify sort fails when input file has wrong format
TEST(sortTest, badInputFileFormat) {
    // Non-existent file
    boost::filesystem::path ouput_path = boost::filesystem::path{ ".\\temp\\test-bad.txt" };

    // Sort the data in a map output file
    Sorting sorter = Sorting();
    int success = sorter.sort(ouput_path);
    EXPECT_EQ(success, -1);
}


// Verify reducer returns correct directory
TEST(reduceTest, checkOutputDirectory) {
    boost::filesystem::path output_directory = boost::filesystem::path{ ".\\temp" };
    Reduce* reducer = createReducer(output_directory);
    EXPECT_EQ(output_directory.compare(reducer->getOutputDirectory()), 0);
    delete reducer;
}

// Verify reducer returns correct output path
TEST(reduceTest, checkOutputPath) {
    boost::filesystem::path output_directory = boost::filesystem::path{ ".\\temp" };
    boost::filesystem::path output_path = boost::filesystem::path{ ".\\temp\\reduce.txt" };
    Reduce* reducer = createReducer(output_directory);
    EXPECT_EQ(output_path.compare(reducer->getOutputPath()), 0);
    delete reducer;
}

// Verify reduce fails when cannot write to disk
TEST(reduceTest, checkBadDirectory) {
    // Non-existent directory
    boost::filesystem::path output_directory = boost::filesystem::path{ ".\\tem" };

    // Example output of sort
    boost::container::map<std::string, std::vector<int>> map_data;
    std::vector<int> one{ 1 };
    std::vector<int> two{ 1, 1 };
    map_data["the"] = two;
    map_data["quick"] = one;
    map_data["brown"] = one;
    map_data["fox"] = one;
    map_data["jumps"] = one;
    map_data["over"] = one;
    map_data["lazy"] = one;
    map_data["dog"] = one;


    int success;
    // Run reduce
    Reduce* reducer = createReducer(output_directory);
    for (auto const& pair : map_data) {
        success = reducer->reduce(pair.first, pair.second);
        EXPECT_EQ(success, -1); // Check reduce fails
    }
    delete reducer;
}

// Verify reduce produces correct ouput
TEST(reduceTest, checkReduceOutput) {
    boost::filesystem::path output_directory = boost::filesystem::path{ ".\\temp" };

    // Example output of sort
    boost::container::map<std::string, std::vector<int>> map_data;
    std::vector<int> one{ 1 };
    std::vector<int> two{ 1, 1 };
    map_data["the"] = two;
    map_data["quick"] = one;
    map_data["brown"] = one;
    map_data["fox"] = one;
    map_data["jumps"] = one;
    map_data["over"] = one;
    map_data["lazy"] = one;
    map_data["dog"] = one;


    int success;
    // Run reduce
    Reduce* reducer = createReducer(output_directory);
    for (auto const& pair : map_data) {
        success = reducer->reduce(pair.first, pair.second);
        EXPECT_EQ(success, 0);
    }

    std::array<std::string, 8> expected_result = {
        "(brown, 1)",
        "(dog, 1)",
        "(fox, 1)",
        "(jumps, 1)",
        "(lazy, 1)",
        "(over, 1)",
        "(quick, 1)",
        "(the, 2)",
    };
    boost::filesystem::ifstream reducer_output(reducer->getOutputPath());
    delete reducer;

    // Verify the output in the file is correct
    std::string line;
    for (int i = 0; i < expected_result.size(); i++) {
        getline(reducer_output, line);
        EXPECT_EQ(line.compare(expected_result[i]), 0);
    };

    reducer_output.close();
}

// Workflow tests
TEST(WorkflowTest, testConstructor)
{

    std::string tar_dir = ".\\shakespeare";
    std::string inter_dir = ".\\temp2";
    std::string out_dir = ".\\output";
    std::string map_dll = ".\\dlls\\MapLibrary.dll";
    std::string reduce_dll = ".\\dlls\\ReduceLibrary.dll";


    Workflow workflow = Workflow(tar_dir, inter_dir, out_dir, map_dll, reduce_dll, 1, 1);

    std::string correct_tar_dir = ".\\shakespeare";
    boost::filesystem::path tar_path = workflow.getTargetDir();
    ASSERT_EQ(tar_path.string(), correct_tar_dir);

    std::string correct_inter_dir = ".\\temp2";
    boost::filesystem::path inter_path = workflow.getIntermediateDir();
    ASSERT_EQ(inter_path.string(), correct_inter_dir);

    std::string correct_out_dir = ".\\output";
    boost::filesystem::path out_path = workflow.getOutDir();
    ASSERT_EQ(out_path.string(), correct_out_dir);

    boost::filesystem::path map_path = workflow.getMapLibPath();
    ASSERT_EQ(map_path.string(), map_dll);

    boost::filesystem::path reduce_path = workflow.getReduceLibPath();
    ASSERT_EQ(reduce_path.string(), reduce_dll);
}

TEST(WorkflowTest, testRun)
{
    std::string tar_dir = ".\\shakespeare";
    std::string inter_dir = ".\\temp2";
    std::string out_dir = ".\\output";
    std::string map_dll = ".\\dlls\\MapLibrary.dll";
    std::string reduce_dll = ".\\dlls\\ReduceLibrary.dll";

    Workflow workflow = Workflow(tar_dir, inter_dir, out_dir, map_dll, reduce_dll, 1, 1);

   workflow.run();
   //boost::filesystem::path success_file = boost::filesystem::path{ out_dir + "\\SUCCESS" };
    //ASSERT_EQ(boost::filesystem::exists(success_file), true);*/

}