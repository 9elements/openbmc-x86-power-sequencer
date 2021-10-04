
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
  
  try
  {
    options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("config", value<string>()->default_value(""), "Path to configuration file/folder.");

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

      cfg = LoadConfig(vm["config"].as<string>());
  }
  catch (const error &ex)
  {
    std::cerr << ex.what() << '\n';
  }

  SignalProvider signalprovider(cfg);
  ACPIStates states(cfg, signalprovider);
  StateMachine sm(cfg, signalprovider);

  sm.Validate();

  sm.Run(); // Run never returns
  return 0;
}
