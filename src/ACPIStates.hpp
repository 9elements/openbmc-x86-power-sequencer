#pragma once

#include "Config.hpp"
#include "Dbus.hpp"
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
class ACPIStates : public SignalDriver, SignalReceiver
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

    ACPIStates(Config& cfg, SignalProvider& sp, boost::asio::io_service& io);
    ~ACPIStates();

    vector<Signal*> Signals(void);

    void Update(void);

  private:
    bool RequestedPowerTransition(const std::string& requested,
                                  std::string& resp);
    bool RequestedHostTransition(const std::string& requested,
                                 std::string& resp);
    SignalProvider* sp;
    enum ACPILevel requestedState;
    unordered_map<enum ACPILevel, Signal*> inputs;
    unordered_map<enum ACPILevel, Signal*> outputs;
    Dbus dbus;
};
