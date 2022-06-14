// mapreduce.cpp
// Todd Hricik
// CIS 687 Object Oriented Design
// Project 1
//
// This program acts as executor which will intantiate a workflow object and invoke 
// the workflow's run member function
//
// June 2, 2022 - Project 3
//  Updated to use Boost's program options for command line args
#include <iostream>
#include <string>
#include <boost\log\core.hpp>
#include <boost\log\expressions.hpp>
#include <boost\filesystem.hpp>
#include <boost\log\trivial.hpp>
#include <boost\program_options.hpp>
#include "executive.h"
#include "workflow.h"

// Set the boost logging threshold to >= info
void init()
{
    boost::log::core::get()->set_filter
    (
        boost::log::trivial::severity >= boost::log::trivial::info
    );
}

int main(int argc, char* argv[])
{
    init();

    std::string input_dir;
    std::string inter_dir;
    std::string output_dir;
    std::string map_dll;
    std::string reduce_dll;
    int map_count;
    int reduce_count;

    // Build argument parser
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Show options")
        ("version", "Verison number")
        ("input", boost::program_options::value<std::string>(), "Input directory path. Required.")
        ("temp", boost::program_options::value<std::string>(), "Intermediate directory path. Required.")
        ("output", boost::program_options::value<std::string>(), "Output directory path. Required.")
        ("map-dll", boost::program_options::value<std::string>(), "Path to map DLL. Required.")
        ("reduce-dll", boost::program_options::value<std::string>(), "Path to reduce DLL. Required.")
        ("mappers", boost::program_options::value<int>(&map_count)->default_value(2), "Number of map threads. Optional. Defaults to 2.")
        ("reducers", boost::program_options::value<int>(&reduce_count)->default_value(3), "Number of reduce threads. Optional. Defaults to 3.")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }


    if (vm.count("version")) {
        std::cout << 3 << "\n";
        return 1;
    }

    // Required arguments
    if (vm.count("input")) {
        input_dir = vm["input"].as<std::string>();
    }
    else {
        BOOST_LOG_TRIVIAL(fatal) << "Missing input directory. An input directory must be provided.";
        return 1;
    }

    if (vm.count("temp")) {
        inter_dir = vm["temp"].as<std::string>();
    }
    else {
        BOOST_LOG_TRIVIAL(fatal) << "Missing temporary file directory. An temporary directory must be provided.";
        return 1;
    }

    if (vm.count("output")) {
        output_dir = vm["output"].as<std::string>();
    }
    else {
        BOOST_LOG_TRIVIAL(fatal) << "Missing output directory. An output directory must be provided.";
        return 1;
    }

    if (vm.count("map-dll")) {
        map_dll = vm["map-dll"].as<std::string>();
    }
    else {
        BOOST_LOG_TRIVIAL(fatal) << "Missing path to map DLL. A map DLL must be provided.";
        return 1;
    }

    if (vm.count("reduce-dll")) {
        reduce_dll = vm["reduce-dll"].as<std::string>();
    }
    else {
        BOOST_LOG_TRIVIAL(fatal) << "Missing path to reduce DLL. A map DLL must be provided.";
        return 1;
    }


    // Optional arguments
    if (vm.count("mappers")) {
        map_count = vm["mappers"].as<int>();
    }

    if (vm.count("reducers")) {
        reduce_count = vm["reducers"].as<int>();
    }


    // Instantiate an executor
    Executive* executive = new Executive(input_dir, inter_dir, output_dir, map_dll, reduce_dll, map_count, reduce_count);

    // Use the executor to run the workflow
    executive->run();

    delete executive;
}