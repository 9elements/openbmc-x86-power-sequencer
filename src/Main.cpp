
#include <boost/program_options.hpp>
#include <iostream>
#include "Config.hpp" 
#include "SignalProvider.hpp" 
#include "ACPIStates.hpp" 
#include "StateMachine.hpp" 

using namespace std;
using namespace boost::program_options;

int main(int argc, const char *argv[]) {
	Config cfg;
	string dumpSignalsFolder;
	boost::asio::io_service io;
	try
	{
		options_description desc{"Options"};
		desc.add_options()
		("help,h", "Help screen")
		("config", value<string>()->default_value(""), "Path to configuration file/folder.")
		("dump_signal_folder", value<string>()->default_value(""), "Path to dump signal.txt [DEBUGGING ONLY]");

		variables_map vm;
		store(parse_command_line(argc, argv, desc), vm);
		notify(vm);

		if (vm.count("help")) {
			std::cout << desc << '\n';
			return 1;
		}
		if (!vm.count("config") || vm["config"].as<string>() == "") {
			std::cout << desc << '\n';
			return 1;
		}
		if (vm.count("dump_signal_folder") ) 
			dumpSignalsFolder = vm["dump_signal_folder"].as<string>();
	
		try {
			cfg = LoadConfig(vm["config"].as<string>());
		} catch (const exception &ex) {
			std::cerr << "Failed to load config:" << std::endl << ex.what() << std::endl;
			return 1;
		}
	} catch (const error &ex) {
		std::cerr << "Failed to parse command line options:" << std::endl << ex.what() << std::endl;
	}

	try {
		SignalProvider signalprovider(cfg, dumpSignalsFolder);
		ACPIStates states(cfg, signalprovider);

		StateMachine sm(cfg, signalprovider, io);

		sm.Validate();
		sm.Run();
	} catch (const exception &ex) {
		std::cerr << "Failed to use provided configuration:" << std::endl << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
