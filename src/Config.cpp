#include <iostream>
#include <filesystem>
#include <boost/algorithm/string/predicate.hpp>
#include "Config.hpp"

namespace fs = std::filesystem;
 
using namespace std;
 
Config LoadConfig(string path) {
  Config cfg;
  for(auto& p: fs::recursive_directory_iterator(path)) {
    if (boost::algorithm::ends_with(std::string(p.path()), std::string(".yaml"))) {
      std::cout << "Loading YAML " << p.path() << std::endl;
      YAML::Node root = YAML::LoadFile(p.path());
      
        Config newConfig = root.as<Config>();
        if (newConfig.Logic.size() > 0) {
          std::cout << " merging " << newConfig.Logic.size() << " logic units into config " << std::endl;
          cfg.Logic.insert(cfg.Logic.end(), newConfig.Logic.begin(), newConfig.Logic.end());
        }
        if (newConfig.Inputs.size() > 0) {
          std::cout << " merging " << newConfig.Inputs.size() << " input units into config " << std::endl;
          cfg.Inputs.insert(cfg.Inputs.end(), newConfig.Inputs.begin(), newConfig.Inputs.end());
        }
        if (newConfig.Outputs.size() > 0) {
          cfg.Outputs.insert(cfg.Outputs.end(), newConfig.Outputs.begin(), newConfig.Outputs.end());
          std::cout << " merging " << newConfig.Outputs.size() << " output units into config " << std::endl;
        }
    }
  }
  return cfg;
}
