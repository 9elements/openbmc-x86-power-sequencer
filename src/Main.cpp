#include <iostream>
#include <ctime>
#include <yaml-cpp/yaml.h>

using namespace std;

struct ConfigLogicOutput {
	string SignalName;
	bool ActiveLow;
};

struct ConfigLogicInput {
	string SignalName;
	bool Invert;
	int InputStableUsec;
};

struct ConfigLogic {
	string Name;
	std::vector<ConfigLogicInput> AndSignalInputs;
	std::vector<ConfigLogicInput> OrSignalInputs;
	bool AndThenOr;
	bool InvertFirstGate;
	int DelayOutputUsec;
	ConfigLogicOutput Out;
};


namespace YAML {
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
  
  static bool decode(const Node& node, std::vector<ConfigLogicInput>& c) {
    if (!node.IsSequence()) {
      return false;
    }
    for (int i = 0; i < node.size(); i++) {
      c.push_back(node[i].as<ConfigLogicInput>());
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
}

namespace YAML {
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
}

int main(void) {
	YAML::Node config = YAML::LoadFile("config/cpu0_pwr.yaml");

	for (YAML::const_iterator it=config.begin();it!=config.end();++it) {
		YAML::Node key = it->first;
		YAML::Node value = it->second;
		std::cout << "Node name = " << key.as<std::string>()  << "\n";
		std::cout << "Node is type" << value.Type()  << "\n";
		if (value.IsSequence()) {
			for (int i = 0; i < value.size(); i++) {
				ConfigLogic c = value[i].as<ConfigLogic>();
				std::cout << "ConfigLogic "  << c.Name << "\n";
				std::cout << "  drives Wire "  << c.Out.SignalName << "\n";
				std::cout << "  AndThenOr "  << c.AndThenOr << "\n";
				std::cout << "  DelayOutputUsec "  << c.DelayOutputUsec << "\n";

			}

		}
	
	}
	return 0;
}
