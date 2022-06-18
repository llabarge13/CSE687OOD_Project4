#include <iostream>
#include <string>
#include <iostream>
#include <functional>
#include <conio.h>
#include <boost\log\core.hpp>
#include <boost\log\expressions.hpp>
#include <boost\filesystem.hpp>
#include <boost\log\trivial.hpp>
#include <boost\program_options.hpp>
#include "executive.h"

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
    //init();

    std::string map_dll;
    std::string reduce_dll;
    std::vector<std::string> stub_endpoints;

    // Build argument parser
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Show options")
        ("map-dll", boost::program_options::value<std::string>(&map_dll), "Path to map DLL. Required.")
        ("reduce-dll", boost::program_options::value<std::string>(&reduce_dll), "Path to reduce DLL. Required.")
        ("stubs", boost::program_options::value<std::vector<std::string>>(&stub_endpoints)->multitoken(), "Endpoint(s) for map/reduce stub(s). Required.")
        ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).style(
        boost::program_options::command_line_style::unix_style ^ boost::program_options::command_line_style::allow_short
    ).run(), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    // Required arguments
    if (!vm.count("map-dll")) {
        BOOST_LOG_TRIVIAL(fatal) << "Missing path to map DLL. A map DLL must be provided.";
        return 1;
    }

    if (!vm.count("reduce-dll")) {
        BOOST_LOG_TRIVIAL(fatal) << "Missing path to reduce DLL. A map DLL must be provided.";
        return 1;
    }

    if (!vm.count("stubs")) {
        BOOST_LOG_TRIVIAL(fatal) << "Missing endpoint(s) for map/reduce stub(s). Endpoint(s) must be provided";
        return 1;
    }

    // Start the stubs
    Executive* executive = new Executive(map_dll, reduce_dll, stub_endpoints);
   // executive->run();

   /* MsgPassingCommunication::EndPoint ep1("localhost", 9191);
    MsgPassingCommunication::Comm comm1(ep1, "comm1");
    comm1.start();

    MsgPassingCommunication::EndPoint ep2("localhost", 9091);


 
    MsgPassingCommunication::Message msg;
    msg.name("msg #1");
    msg.to(ep2);
    msg.from(ep1);
    comm1.postMessage(msg);*/

   // delete executive;

    while (true);
}