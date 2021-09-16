#include <iostream>
#include <ctime>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <boost/algorithm/string/predicate.hpp>

namespace fs = std::filesystem;
 

using namespace std;

struct ConfigLogicOutput {
        // The signal that is driven by this block
        string SignalName;
        // ActiveLow inverts the logic level of the signal when set
        bool ActiveLow;
};

struct ConfigLogicInput {
        // The signal where the logic level is read from
        string SignalName;
        // Invert invertes the input signal level
        bool Invert;
        // InputStableUsec specifies the time where the input level needs to
        // be constant
        int InputStableUsec;
};

struct ConfigLogic {
        // Name of the logic Unit
        string Name;
        // List of input signals that are logical ANDed
        std::vector<ConfigLogicInput> AndSignalInputs;
        // List of input signals that are logical ORed
        std::vector<ConfigLogicInput> OrSignalInputs;
        // AndThenOr gives the relation between AND and OR gates
        //      True:             False
        // - |                  -------- |
        // - | AND - |          -------- | AND -
        // - |       |          -------- |
        //           | OR -              |
        // ----------|          - | OR - |
        // ----------|          - |
        bool AndThenOr;
        // InvertFirstGate turns the first logical gate into
        // and NAND or an NOR gatter (depending which is the first).
        bool InvertFirstGate;
        // DelayOutputUsec adds a fixed delay to every level change
        int DelayOutputUsec;
        // Out configures which Signal is driven by this logic block
        ConfigLogicOutput Out;
};

enum ConfigInputType {
        INPUT_TYPE_GPIO,
};

enum ConfigOutputType {
        OUTPUT_TYPE_GPIO,
};

struct ConfigInput {
        // The name where the signal level is read from
        string Name;
        // The signal where the logic level is applied to
        string SignalName;
        // ActiveLow invertes the input signal level
        bool ActiveLow;
        // Description is just for debugging purposes
        string Description;
        // Type specifies the input backend to use
        enum ConfigInputType InputType;
};

struct ConfigOutput {
        // The name where the signal level is appied to
        string Name;
        // The signal where the logic level is read from
        string SignalName;
        // ActiveLow invertes the input signal level
        bool ActiveLow;
        // Description is just for debugging purposes
        string Description;
        // Type specifies the output backend to use
        enum ConfigOutputType InputType;
};

struct Config {
        std::vector<ConfigLogic> Logic;
        std::vector<ConfigInput> Inputs;
        std::vector<ConfigOutput> Outputs;
};

namespace YAML {
template<>
struct convert<ConfigOutput> {
  
  static bool decode(const Node& node, ConfigOutput& c) {
    if (!node.IsMap()) {
      return false;
    }

    YAML::Node key = node.begin()->first;
    YAML::Node value = node.begin()->second;
    c.Name = key.as<std::string>();
    
    for (auto it=value.begin();it!=value.end();++it) {
      if (it->first.as<std::string>().compare("name") == 0) {
        c.SignalName = it->second.as<string>();     
      } else if (it->first.as<std::string>().compare("description") == 0) {
        c.Description = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("active_low") == 0) {
        c.ActiveLow = it->second.as<bool>();
      } else if (it->first.as<std::string>().compare("type") == 0) {
        string nameOfType = it->second.as<string>();
        if (nameOfType.compare("gpio") == 0) {
          c.InputType = OUTPUT_TYPE_GPIO;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
    return true;
  }
};

template<>
struct convert<std::vector<ConfigOutput>> {
  
  static bool decode(const Node& node, std::vector<ConfigOutput>& v) {
    if (!node.IsSequence()) {
      return false;
    }
    for (int i = 0; i < node.size(); i++) {
      v.push_back(node[i].as<ConfigOutput>());
    }

    return true;
  }
};

template<>
struct convert<ConfigInput> {
  
  static bool decode(const Node& node, ConfigInput& c) {
    if (!node.IsMap()) {
      return false;
    }

    YAML::Node key = node.begin()->first;
    YAML::Node value = node.begin()->second;
    c.Name = key.as<std::string>();
    
    for (auto it=value.begin();it!=value.end();++it) {
      if (it->first.as<std::string>().compare("name") == 0) {
        c.SignalName = it->second.as<string>();     
      } else if (it->first.as<std::string>().compare("description") == 0) {
        c.Description = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("active_low") == 0) {
        c.ActiveLow = it->second.as<bool>();
      } else if (it->first.as<std::string>().compare("type") == 0) {
        string nameOfType = it->second.as<string>();
        if (nameOfType.compare("gpio") == 0) {
          c.InputType = INPUT_TYPE_GPIO;
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
    return true;
  }
};

template<>
struct convert<std::vector<ConfigInput>> {
  
  static bool decode(const Node& node, std::vector<ConfigInput>& v) {
    if (!node.IsSequence()) {
      return false;
    }
    for (int i = 0; i < node.size(); i++) {
      v.push_back(node[i].as<ConfigInput>());
    }

    return true;
  }
};


template<>
struct convert<ConfigLogicOutput> {
  
  static bool decode(const Node& node, ConfigLogicOutput& c) {
    if (!node.IsMap()) {
      return false;
    }

    for (auto it=node.begin();it!=node.end();++it) {
       std::string name = it->first.as<std::string>();
       if (name.compare("name") == 0) {
        c.SignalName = it->second.as<std::string>();
       } else if (name.compare("active_low") == 0) {
        c.ActiveLow = it->second.as<bool>();
       } else {
         return false;
       }
    }
   
    return true;
  }
};

template<>
struct convert<std::vector<ConfigLogicInput>> {
  
  static bool decode(const Node& node, std::vector<ConfigLogicInput>& v) {
    if (!node.IsSequence()) {
      return false;
    }
    for (int i = 0; i < node.size(); i++) {
      v.push_back(node[i].as<ConfigLogicInput>());
    }

    return true;
  }
};

template<>
struct convert<ConfigLogicInput> {
  
  static bool decode(const Node& node, ConfigLogicInput& c) {
    if (!node.IsMap()) {
      return false;
    }

    for (auto it=node.begin();it!=node.end();++it) {
       std::string name = it->first.as<std::string>();
       if (name.compare("name") == 0) {
        c.SignalName = it->second.as<std::string>();
       } else if (name.compare("invert") == 0) {
        c.Invert = it->second.as<bool>();
       } else if (name.compare("input_stable_usec") == 0) {
        c.InputStableUsec = it->second.as<int>();
       } else {
         return false;
       }
    }
   
    return true;
  }
};

template<>
struct convert<ConfigLogic> {
  
  static bool decode(const Node& node, ConfigLogic& c) {
    if (!node.IsMap()) {
      return false;
    }
  
    YAML::Node key = node.begin()->first;
    YAML::Node value = node.begin()->second;
    c.Name = key.as<std::string>();
    
    for (auto it=value.begin();it!=value.end();++it) {
      if (it->first.as<std::string>().compare("in") == 0) {
        YAML::Node inmap = it->second;
        if (inmap.IsMap()) {
          for (auto it2=inmap.begin();it2!=inmap.end();++it2) {
            if (it2->first.as<std::string>().compare("and") == 0) {
                c.AndSignalInputs = it2->second.as<std::vector<ConfigLogicInput>>();     
            } else if (it2->first.as<std::string>().compare("or") == 0) {
                c.OrSignalInputs = it2->second.as<std::vector<ConfigLogicInput>>();     
            } else {
              return false;
            }
          }
        }
      } else if (it->first.as<std::string>().compare("out") == 0) {
              c.Out = it->second.as<ConfigLogicOutput>();     
      } else if (it->first.as<std::string>().compare("and_then_or") == 0) {
              c.AndThenOr = it->second.as<bool>();
      } else if (it->first.as<std::string>().compare("delay_usec") == 0) {
              c.DelayOutputUsec = it->second.as<int>();
      }
    }
   
    return true;
  }
};

template<>
struct convert<std::vector<ConfigLogic>> {
  
  static bool decode(const Node& node, std::vector<ConfigLogic>& v) {
    if (!node.IsSequence()) {
      return false;
    }
    for (int i = 0; i < node.size(); i++) {
      v.push_back(node[i].as<ConfigLogic>());
    }
   
    return true;
  }
};


template<>
struct convert<Config> {
  
  static bool decode(const Node& node, Config& c) {
    if (!node.IsMap()) {
      return false;
    }

    for (auto it=node.begin();it!=node.end();++it) {
	        	std::cout << "Node2 " << it->first.as<std::string>() << std::endl;

      if (it->first.as<std::string>().compare("power_sequencer") == 0) {
        std::vector<ConfigLogic> newLogic = it->second.as<std::vector<ConfigLogic>>();    
	c.Logic.insert(c.Logic.end(), newLogic.begin(), newLogic.end());
      } else if (it->first.as<std::string>().compare("inputs") == 0) {
        std::vector<ConfigInput> newInputs = it->second.as<std::vector<ConfigInput>>();
	c.Inputs.insert(c.Inputs.end(), newInputs.begin(), newInputs.end());
      } else if (it->first.as<std::string>().compare("outputs") == 0) {
       std::vector<ConfigOutput> newOutputs = it->second.as<std::vector<ConfigOutput>>();    
	c.Outputs.insert(c.Outputs.end(), newOutputs.begin(), newOutputs.end());
      } 
    }
   
    return true;
  }
};
}


int main(void) {
	std::stringstream buffer;

  for(auto& p: fs::recursive_directory_iterator("config")) {
    if (boost::algorithm::ends_with(std::string(p.path()), std::string(".yaml"))) {
      std::ifstream t(p.path());
      buffer << t.rdbuf();
    }
  }
  cout << buffer.str() << std::endl;
  YAML::Node config = YAML::Load(buffer.str());
  Config c = config.as<Config>();
for (int i = 0; i < c.Logic.size(); i++) {
	std::cout << "Node " << i << std::endl;
	std::cout << " Name: " << c.Logic[i].Name << std::endl;
	std::cout << " Signal: " << c.Logic[i].Out.SignalName << std::endl;
}
#if 0
        for (YAML::const_iterator it=config.begin();it!=config.end();++it) {
                YAML::Node key = it->first;
                YAML::Node value = it->second;
                std::cout << "Node name = " << key.as<std::string>()  << "\n";
                std::cout << "Node is type" << value.Type()  << "\n";
                if (key.as<std::string>().compare("power_sequencer") == 0) {
                        std::vector<ConfigLogic> vec = value.as<std::vector<ConfigLogic>>();
			for (int i = 0; i < vec.size(); i++) {
				 std::cout << "Node " << i << std::endl;
				 std::cout << " Name: " << vec[0].Name << std::endl;
				 std::cout << " Signal: " << vec[0].Out.SignalName << std::endl;
			}
                }
        }
	#endif
        return 0;
}
