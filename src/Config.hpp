
#ifndef _CONFIG_HPP__
#define _CONFIG_HPP__

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <vector>

using namespace std;

struct ConfigLogicOutput
{
    // The signal that is driven by this block
    string SignalName;
    // ActiveLow inverts the logic level of the signal when set
    bool ActiveLow;
};

struct ConfigLogicInput
{
    // The signal where the logic level is read from
    string SignalName;
    // Invert invertes the input signal level
    bool Invert;
    // InputStableUsec specifies the time where the input level needs to
    // be constant
    int InputStableUsec;
};

struct ConfigLogic
{
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

enum ConfigInputType
{
    INPUT_TYPE_GPIO,
    INPUT_TYPE_NULL,
    INPUT_TYPE_UNKNOWN,
};

enum ConfigOutputType
{
    OUTPUT_TYPE_GPIO,
    OUTPUT_TYPE_NULL,
    OUTPUT_TYPE_UNKNOWN,
};

struct ConfigInput
{
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

struct ConfigOutput
{
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

struct ConfigRegulator
{
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

struct ConfigImmutable
{
    // The signal where the logic level is applied to
    string SignalName;
    // Level is the level the signal always drives
    bool Level;
};

struct ConfigACPIStates
{
    // Name is the name of the ACPI state
    std::string Name;
    // Initial is true for the ACPI state at power on
    bool Initial;
};

struct Config
{
    std::vector<ConfigLogic> Logic;
    std::vector<ConfigInput> Inputs;
    std::vector<ConfigOutput> Outputs;
    std::vector<std::string> FloatingSignals;
    std::vector<ConfigRegulator> Regulators;
    std::vector<ConfigImmutable> Immutables;
    std::vector<ConfigACPIStates> ACPIStates;
};

Config LoadConfig(string path);

#endif