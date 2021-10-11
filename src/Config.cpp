#include "Config.hpp"

#include "Logging.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

using namespace std;

namespace YAML
{
template <>
struct convert<ConfigOutput>
{

    static bool decode(const Node& node, ConfigOutput& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        c.OutputType = OUTPUT_TYPE_UNKNOWN;
        c.Name = "";
        c.SignalName = "";
        c.Description = "";
        c.GpioChipName = "";
        c.ActiveLow = false;
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            if (it->first.as<std::string>().compare("name") == 0)
            {
                c.Name = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("signal_name") == 0)
            {
                c.SignalName = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("description") == 0)
            {
                c.Description = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("gpio_chip_name") == 0)
            {
                c.GpioChipName = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("active_low") == 0)
            {
                c.ActiveLow = it->second.as<bool>();
            }
            else if (it->first.as<std::string>().compare("type") == 0)
            {
                string nameOfType = it->second.as<string>();
                if (nameOfType.compare("gpio") == 0)
                {
                    c.OutputType = OUTPUT_TYPE_GPIO;
                }
                else if (nameOfType.compare("null") == 0)
                {
                    c.OutputType = OUTPUT_TYPE_NULL;
                }
                else
                {
                    return false;
                }
            }
        }
        if (c.OutputType == OUTPUT_TYPE_UNKNOWN || c.Name == "" ||
            c.SignalName == "")
            return false;
        return true;
    }
};

template <>
struct convert<ConfigInput>
{

    static bool decode(const Node& node, ConfigInput& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        c.InputType = INPUT_TYPE_UNKNOWN;
        c.Name = "";
        c.SignalName = "";
        c.Description = "";
        c.GpioChipName = "";
        c.ActiveLow = false;

        for (auto it = node.begin(); it != node.end(); ++it)
        {

            if (it->first.as<std::string>().compare("name") == 0)
            {
                c.Name = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("signal_name") == 0)
            {
                c.SignalName = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("description") == 0)
            {
                c.Description = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("gpio_chip_name") == 0)
            {
                c.GpioChipName = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("active_low") == 0)
            {
                c.ActiveLow = it->second.as<bool>();
            }
            else if (it->first.as<std::string>().compare("type") == 0)
            {
                string nameOfType = it->second.as<string>();
                if (nameOfType.compare("gpio") == 0)
                {
                    c.InputType = INPUT_TYPE_GPIO;
                }
                else if (nameOfType.compare("null") == 0)
                {
                    c.InputType = INPUT_TYPE_NULL;
                }
                else
                {
                    return false;
                }
            }
        }
        if (c.InputType == INPUT_TYPE_UNKNOWN || c.Name == "" ||
            c.SignalName == "")
            return false;

        return true;
    }
};

template <>
struct convert<ConfigLogicOutput>
{

    static bool decode(const Node& node, ConfigLogicOutput& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        c.ActiveLow = false;
        c.SignalName = "";

        for (auto it = node.begin(); it != node.end(); ++it)
        {
            std::string name = it->first.as<std::string>();
            if (name.compare("name") == 0)
            {
                c.SignalName = it->second.as<std::string>();
            }
            else if (name.compare("active_low") == 0)
            {
                c.ActiveLow = it->second.as<bool>();
            }
            else
            {
                return false;
            }
        }
        if (c.SignalName == "")
            return false;

        return true;
    }
};

template <>
struct convert<std::vector<ConfigLogicInput>>
{

    static bool decode(const Node& node, std::vector<ConfigLogicInput>& v)
    {
        if (!node.IsSequence())
        {
            return false;
        }
        for (int i = 0; i < node.size(); i++)
        {
            v.push_back(node[i].as<ConfigLogicInput>());
        }

        return true;
    }
};

template <>
struct convert<ConfigLogicInput>
{

    static bool decode(const Node& node, ConfigLogicInput& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        c.InputStableUsec = 0;
        c.Invert = false;
        c.SignalName = "";

        for (auto it = node.begin(); it != node.end(); ++it)
        {
            std::string name = it->first.as<std::string>();
            if (name.compare("name") == 0)
            {
                c.SignalName = it->second.as<std::string>();
            }
            else if (name.compare("invert") == 0)
            {
                c.Invert = it->second.as<bool>();
            }
            else if (name.compare("input_stable_usec") == 0)
            {
                c.InputStableUsec = it->second.as<int>();
            }
            else
            {
                return false;
            }
        }
        if (c.SignalName == "")
            return false;

        return true;
    }
};

template <>
struct convert<ConfigLogic>
{

    static bool decode(const Node& node, ConfigLogic& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        YAML::Node key = node.begin()->first;
        YAML::Node value = node.begin()->second;
        c.Name = key.as<std::string>();
        c.AndThenOr = false;
        c.InvertFirstGate = false;
        c.DelayOutputUsec = 0;

        for (auto it = value.begin(); it != value.end(); ++it)
        {
            if (it->first.as<std::string>().compare("in") == 0)
            {
                YAML::Node inmap = it->second;
                if (inmap.IsMap())
                {
                    for (auto it2 = inmap.begin(); it2 != inmap.end(); ++it2)
                    {
                        if (it2->first.as<std::string>().compare("and") == 0)
                        {
                            c.AndSignalInputs =
                                it2->second.as<std::vector<ConfigLogicInput>>();
                        }
                        else if (it2->first.as<std::string>().compare("or") ==
                                 0)
                        {
                            c.OrSignalInputs =
                                it2->second.as<std::vector<ConfigLogicInput>>();
                        }
                        else if (it2->first.as<std::string>().compare(
                                     "and_then_or") == 0)
                        {
                            c.AndThenOr = it2->second.as<bool>();
                        }
                        else if (it2->first.as<std::string>().compare(
                                     "invert_first_gate") == 0)
                        {
                            c.InvertFirstGate = it2->second.as<bool>();
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
            }
            else if (it->first.as<std::string>().compare("out") == 0)
            {
                c.Out = it->second.as<ConfigLogicOutput>();
            }
            else if (it->first.as<std::string>().compare("delay_usec") == 0)
            {
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

template <>
struct convert<ConfigImmutable>
{

    static bool decode(const Node& node, ConfigImmutable& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        c.SignalName = "";
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            if (it->first.as<std::string>().compare("signal_name") == 0)
            {
                c.SignalName = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("level") == 0)
            {
                c.Level = it->second.as<bool>();
            }
        }
        if (c.SignalName == "")
            return false;

        return true;
    }
};

template <>
struct convert<ConfigRegulator>
{

    static bool decode(const Node& node, ConfigRegulator& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        c.Name = "";

        for (auto it = node.begin(); it != node.end(); ++it)
        {
            if (it->first.as<std::string>().compare("name") == 0)
            {
                c.Name = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("description") == 0)
            {
                // FIXME
            }
            else if (it->first.as<std::string>().compare("vout") == 0)
            {
                c.vout = it->second.as<float>();
            }
            else if (it->first.as<std::string>().compare(
                         "vout_uv_fault_limit") == 0)
            {
                c.vout_uv_fault_limit = it->second.as<float>();
            }
            else if (it->first.as<std::string>().compare(
                         "vout_ov_fault_limit") == 0)
            {
                c.vout_ov_fault_limit = it->second.as<float>();
            }
            else if (it->first.as<std::string>().compare(
                         "vin_ov_fault_limit") == 0)
            {
                c.vin_ov_fault_limit = it->second.as<float>();
            }
            else if (it->first.as<std::string>().compare(
                         "ton_max_fault_limit_msec") == 0)
            {
                c.ton_max_fault_limit_msec = it->second.as<int>();
            }
            else if (it->first.as<std::string>().compare("fault_suppress_us") ==
                     0)
            {
                c.ton_max_fault_limit_msec = it->second.as<int>();
            }
            else if (it->first.as<std::string>().compare("always_on") == 0)
            {
                c.always_on = it->second.as<bool>();
            }
        }
        if (c.Name == "" || c.vout == 0.0f)
        {
            return false;
        }

        return true;
    }
};

template <>
struct convert<ConfigACPIStates>
{

    static bool decode(const Node& node, ConfigACPIStates& c)
    {
        if (!node.IsMap())
        {
            return false;
        }

        c.Name = "";
        for (auto it = node.begin(); it != node.end(); ++it)
        {
            if (it->first.as<std::string>().compare("name") == 0)
            {
                c.Name = it->second.as<string>();
            }
            else if (it->first.as<std::string>().compare("initial") == 0)
            {
                c.Initial = it->second.as<bool>();
            }
        }
        if (c.Name == "")
            return false;

        return true;
    }
};

template <>
struct convert<Config>
{

    static bool decode(const Node& node, Config& c)
    {
        if (!node.IsMap())
        {
            return true;
        }

        for (auto it = node.begin(); it != node.end(); ++it)
        {
            if (it->first.as<std::string>().compare("power_sequencer") == 0)
            {
                std::vector<ConfigLogic> newLogic =
                    it->second.as<std::vector<ConfigLogic>>();
                c.Logic.insert(c.Logic.end(), newLogic.begin(), newLogic.end());
            }
            else if (it->first.as<std::string>().compare("inputs") == 0)
            {
                std::vector<ConfigInput> newInputs =
                    it->second.as<std::vector<ConfigInput>>();
                c.Inputs.insert(c.Inputs.end(), newInputs.begin(),
                                newInputs.end());
            }
            else if (it->first.as<std::string>().compare("outputs") == 0)
            {
                std::vector<ConfigOutput> newOutputs =
                    it->second.as<std::vector<ConfigOutput>>();
                c.Outputs.insert(c.Outputs.end(), newOutputs.begin(),
                                 newOutputs.end());
            }
            else if (it->first.as<std::string>().compare("floating_signals") ==
                     0)
            {
                std::vector<std::string> signals =
                    it->second.as<std::vector<std::string>>();
                c.FloatingSignals.insert(c.FloatingSignals.end(),
                                         signals.begin(), signals.end());
            }
            else if (it->first.as<std::string>().compare("regulators") == 0)
            {
                std::vector<ConfigRegulator> regs =
                    it->second.as<std::vector<ConfigRegulator>>();
                c.Regulators.insert(c.Regulators.end(), regs.begin(),
                                    regs.end());
            }
            else if (it->first.as<std::string>().compare("immutables") == 0)
            {
                std::vector<ConfigImmutable> imm =
                    it->second.as<std::vector<ConfigImmutable>>();
                c.Immutables.insert(c.Immutables.end(), imm.begin(), imm.end());
            }
            else if (it->first.as<std::string>().compare("states") == 0)
            {
                std::vector<ConfigACPIStates> states =
                    it->second.as<std::vector<ConfigACPIStates>>();
                c.ACPIStates.insert(c.ACPIStates.end(), states.begin(),
                                    states.end());
            }
        }

        return true;
    }
};
} // namespace YAML

Config LoadConfig(string path)
{
    Config cfg;
    for (auto& p : fs::recursive_directory_iterator(path))
    {
        if (boost::algorithm::ends_with(std::string(p.path()),
                                        std::string(".yaml")) ||
            boost::algorithm::ends_with(std::string(p.path()),
                                        std::string(".yml")))
        {
            LOGINFO("Loading YAML config " + p.path() + "\n");
            YAML::Node root = YAML::LoadFile(p.path());

            Config newConfig = root.as<Config>();
            if (newConfig.Logic.size() > 0)
            {
                LOGDEBUG("merging " + to_string(newConfig.Logic.size()) +
                         " logic units into config\n");
                cfg.Logic.insert(cfg.Logic.end(), newConfig.Logic.begin(),
                                 newConfig.Logic.end());
            }
            if (newConfig.Inputs.size() > 0)
            {
                LOGDEBUG("merging " + to_string(newConfig.Inputs.size()) +
                         " input units into config\n");
                cfg.Inputs.insert(cfg.Inputs.end(), newConfig.Inputs.begin(),
                                  newConfig.Inputs.end());
            }
            if (newConfig.Outputs.size() > 0)
            {
                cfg.Outputs.insert(cfg.Outputs.end(), newConfig.Outputs.begin(),
                                   newConfig.Outputs.end());
                LOGDEBUG("merging " + to_string(newConfig.Outputs.size()) +
                         " output units into config\n");
            }
            if (newConfig.Regulators.size() > 0)
            {
                cfg.Regulators.insert(cfg.Regulators.end(),
                                      newConfig.Regulators.begin(),
                                      newConfig.Regulators.end());
                LOGDEBUG("merging " + to_string(newConfig.Regulators.size()) +
                         " regulator units into config\n");
            }
            if (newConfig.Immutables.size() > 0)
            {
                cfg.Immutables.insert(cfg.Immutables.end(),
                                      newConfig.Immutables.begin(),
                                      newConfig.Immutables.end());
                LOGDEBUG("merging " + to_string(newConfig.Immutables.size()) +
                         " immutables units into config\n");
            }
            if (newConfig.ACPIStates.size() > 0)
            {
                cfg.ACPIStates.insert(cfg.ACPIStates.end(),
                                      newConfig.ACPIStates.begin(),
                                      newConfig.ACPIStates.end());
                LOGDEBUG("merging " + to_string(newConfig.ACPIStates.size()) +
                         " ACPI state units into config\n");
            }
            if (newConfig.FloatingSignals.size() > 0)
            {
                cfg.FloatingSignals.insert(cfg.FloatingSignals.end(),
                                           newConfig.FloatingSignals.begin(),
                                           newConfig.FloatingSignals.end());
                LOGDEBUG("merging " +
                         to_string(newConfig.FloatingSignals.size()) +
                         " floating signals into config\n");
            }
        }
    }
    return cfg;
}
