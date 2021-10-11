#ifndef _ACPISTATES_HPP__
#define _ACPISTATES_HPP__

#include "Config.hpp"
#include "IODriver.hpp"
#include "Signal.hpp"
#include "SignalProvider.hpp"

#include <unordered_map>
#include <vector>

using namespace std;

enum ACPILevel
{
    ACPI_INVALID,
    ACPI_G3,
    ACPI_S5,
    ACPI_S4,
    ACPI_S3,
    ACPI_S0,
};

// The ACPIStates class handles the ACPI states
class ACPIStates : public SignalDriver
{
  public:
    // Request tells the logic which on is the desired power state
    // The power logic and platform might take a while to change the
    // power state, or even fail.
    void Request(enum ACPILevel l);

    // GetRequested returns the requested ACPI state.
    enum ACPILevel GetRequested(void);

    // GetCurrent returns the current ACPI state.
    enum ACPILevel GetCurrent(void);

    ACPIStates(Config& cfg, SignalProvider& sp);
    ~ACPIStates();

    vector<Signal*> Signals(void);

  private:
    SignalProvider* sp;
    enum ACPILevel requestedState;
    unordered_map<enum ACPILevel, Signal*> inputs;
    unordered_map<enum ACPILevel, Signal*> outputs;
};

#endif