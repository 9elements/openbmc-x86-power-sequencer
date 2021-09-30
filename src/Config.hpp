
#ifndef _CONFIG_HPP__
#define _CONFIG_HPP__

#include <iostream>
#include <vector>
#include <yaml-cpp/yaml.h>

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
	INPUT_TYPE_NULL,
        INPUT_TYPE_UNKNOWN,
};

enum ConfigOutputType {
        OUTPUT_TYPE_GPIO,
	OUTPUT_TYPE_NULL,
        OUTPUT_TYPE_UNKNOWN,
};

struct ConfigInput {
        // The name where the signal level is read from
        string Name;
	// The chipname where the signal level is read from (only for GPIOs)
	string GpioChipName;
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
	// The chipname where the signal level is read from (only for GPIOs)
	string GpioChipName;
        // The signal where the logic level is read from
        string SignalName;
        // ActiveLow invertes the input signal level
        bool ActiveLow;
        // Description is just for debugging purposes
        string Description;
        // Type specifies the output backend to use
        enum ConfigOutputType OutputType;
};

struct ConfigRegulator {
        // The regulator name
        string Name;
        // Description is just for debugging purposes
        string Description;
	// The output voltage when enabled
	float vout;
	// Undervoltage limit driving fault
	float vout_uv_fault_limit;
	// Overvoltage limit driving fault
	float vout_ov_fault_limit;
	// Input overvoltage limit driving fault
	float vin_ov_fault_limit;
	// Time to wait to assert fault after POR
	int ton_max_fault_limit_msec;
	// Suppress fault after fault condition is cleared
	int fault_suppress_us;
	// Always on regulators must not be disabled
	bool always_on;
};

struct ConfigImmutable {
        // The signal where the logic level is applied to
        string SignalName;
        // Level is the level the signal always drives
        bool Level;
};


struct Config {
        std::vector<ConfigLogic> Logic;
        std::vector<ConfigInput> Inputs;
        std::vector<ConfigOutput> Outputs;
	std::vector<std::string> FloatingSignals;
	std::vector<ConfigRegulator> Regulators;
	std::vector<ConfigImmutable> Immutables;
};

namespace YAML {
template<>
struct convert<ConfigOutput> {
  
  static bool decode(const Node& node, ConfigOutput& c) {
    if (!node.IsMap()) {
      return false;
    }

    c.OutputType = OUTPUT_TYPE_UNKNOWN;
    for (auto it=node.begin();it!=node.end();++it) {
      if (it->first.as<std::string>().compare("name") == 0) {
        c.Name = it->second.as<string>();     
      } else if (it->first.as<std::string>().compare("signal_name") == 0) {
        c.SignalName = it->second.as<string>();     
      } else if (it->first.as<std::string>().compare("description") == 0) {
        c.Description = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("gpio_chip_name") == 0) {
        c.GpioChipName = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("active_low") == 0) {
        c.ActiveLow = it->second.as<bool>();
      } else if (it->first.as<std::string>().compare("type") == 0) {
        string nameOfType = it->second.as<string>();
        if (nameOfType.compare("gpio") == 0) {
          c.OutputType = OUTPUT_TYPE_GPIO;
        } else if (nameOfType.compare("null") == 0) {
          c.OutputType = OUTPUT_TYPE_NULL;
        } else {
          return false;
        }
      }
    }
    if (c.OutputType == OUTPUT_TYPE_UNKNOWN)
	    return false;
    return true;
  }
};

#if 0
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
#endif

template<>
struct convert<ConfigInput> {
  
  static bool decode(const Node& node, ConfigInput& c) {
    if (!node.IsMap()) {
      return false;
    }

    c.InputType = INPUT_TYPE_UNKNOWN;
    for (auto it=node.begin();it!=node.end();++it) {

      if (it->first.as<std::string>().compare("name") == 0) {
        c.Name = it->second.as<string>();     
      } else if (it->first.as<std::string>().compare("signal_name") == 0) {
        c.SignalName = it->second.as<string>();     
      } else if (it->first.as<std::string>().compare("description") == 0) {
        c.Description = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("gpio_chip_name") == 0) {
        c.GpioChipName = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("active_low") == 0) {
        c.ActiveLow = it->second.as<bool>();
      } else if (it->first.as<std::string>().compare("type") == 0) {
        string nameOfType = it->second.as<string>();
        if (nameOfType.compare("gpio") == 0) {
          c.InputType = INPUT_TYPE_GPIO;
        } else if (nameOfType.compare("null") == 0) {
          c.InputType = INPUT_TYPE_NULL;
        } else {
          return false;
        }
      }
    }
    if (c.InputType == INPUT_TYPE_UNKNOWN)
	    return false;

    return true;
  }
};
#if 0
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
#endif

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
            } else if (it2->first.as<std::string>().compare("and_then_or") == 0) {
              c.AndThenOr = it2->second.as<bool>();
            } else if (it2->first.as<std::string>().compare("invert_first_gate") == 0) {
              c.InvertFirstGate = it2->second.as<bool>();
            } else {
              return false;
            }
          }
        }
      } else if (it->first.as<std::string>().compare("out") == 0) {
              c.Out = it->second.as<ConfigLogicOutput>();     
      } else if (it->first.as<std::string>().compare("delay_usec") == 0) {
              c.DelayOutputUsec = it->second.as<int>();
      }
    }
  
    return true;
  }
};

#if 0
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
#endif


template<>
struct convert<ConfigImmutable> {
  
  static bool decode(const Node& node, ConfigImmutable& c) {
    if (!node.IsMap()) {
      return false;
    }

    c.SignalName = "";
    for (auto it=node.begin();it!=node.end();++it) {
      if (it->first.as<std::string>().compare("signal_name") == 0) {
        c.SignalName = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("level") == 0) {
         c.Level = it->second.as<bool>();
      }
    }
    if (c.SignalName == "")
      return false;

    return true;
  }
};

template<>
struct convert<ConfigRegulator> {
  
  static bool decode(const Node& node, ConfigRegulator& c) {
    if (!node.IsMap()) {
      return false;
    }
  
    YAML::Node key = node.begin()->first;
    YAML::Node value = node.begin()->second;
  
    for (auto it=value.begin();it!=value.end();++it) {
      if (it->first.as<std::string>().compare("name") == 0) {
        c.Name = it->second.as<string>();
      } else if (it->first.as<std::string>().compare("description") == 0) {
             //FIXME
      } else if (it->first.as<std::string>().compare("vout") == 0) {
              c.vout = it->second.as<float>();
      } else if (it->first.as<std::string>().compare("vout_uv_fault_limit") == 0) {
              c.vout_uv_fault_limit = it->second.as<float>();
      } else if (it->first.as<std::string>().compare("vout_ov_fault_limit") == 0) {
              c.vout_ov_fault_limit = it->second.as<float>();
      } else if (it->first.as<std::string>().compare("vin_ov_fault_limit") == 0) {
              c.vin_ov_fault_limit = it->second.as<float>();
      } else if (it->first.as<std::string>().compare("ton_max_fault_limit_msec") == 0) {
              c.ton_max_fault_limit_msec = it->second.as<int>();
      } else if (it->first.as<std::string>().compare("fault_suppress_us") == 0) {
              c.ton_max_fault_limit_msec = it->second.as<int>();
      } else if (it->first.as<std::string>().compare("always_on") == 0) {
              c.always_on = it->second.as<bool>();
      }
    }
  
    return true;
  }
};

template<>
struct convert<Config> {
  
  static bool decode(const Node& node, Config& c) {
    if (!node.IsMap()) {
      return true;
    }

    for (auto it=node.begin();it!=node.end();++it) {
      if (it->first.as<std::string>().compare("power_sequencer") == 0) {
        std::vector<ConfigLogic> newLogic = it->second.as<std::vector<ConfigLogic>>();    
        c.Logic.insert(c.Logic.end(), newLogic.begin(), newLogic.end());
      } else if (it->first.as<std::string>().compare("inputs") == 0) {
        std::vector<ConfigInput> newInputs = it->second.as<std::vector<ConfigInput>>();
        c.Inputs.insert(c.Inputs.end(), newInputs.begin(), newInputs.end());
      } else if (it->first.as<std::string>().compare("outputs") == 0) {
       std::vector<ConfigOutput> newOutputs = it->second.as<std::vector<ConfigOutput>>();    
        c.Outputs.insert(c.Outputs.end(), newOutputs.begin(), newOutputs.end());
      } else if (it->first.as<std::string>().compare("floating_signals") == 0) {
       std::vector<std::string> signals = it->second.as<std::vector<std::string>>();    
        c.FloatingSignals.insert(c.FloatingSignals.end(), signals.begin(), signals.end());
      } else if (it->first.as<std::string>().compare("regulators") == 0) {
       std::vector<ConfigRegulator> regs = it->second.as<std::vector<ConfigRegulator>>();    
        c.Regulators.insert(c.Regulators.end(), regs.begin(), regs.end());
      } else if (it->first.as<std::string>().compare("immutables") == 0) {
       std::vector<ConfigImmutable> imm = it->second.as<std::vector<ConfigImmutable>>();    
        c.Immutables.insert(c.Immutables.end(), imm.begin(), imm.end());
      } 

      
    }
   
    return true;
  }
};
}

Config LoadConfig(string path);

#endif