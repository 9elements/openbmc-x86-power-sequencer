#ifndef _ACPISTATES_HPP__
#define _ACPISTATES_HPP__

#include <vector>
#include <unordered_map>

#include "Signal.hpp"
#include "Config.hpp"
#include "IODriver.hpp"
#include "SignalProvider.hpp"

using namespace std;

enum ACPILevel {
  ACPI_INVALID,
  ACPI_G3,
  ACPI_S5,
  ACPI_S4,
  ACPI_S3,
  ACPI_S0,
};


// The ACPIStates class handles the ACPI states
class ACPIStates : public InputDriver {
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

	std::vector<Signal *> Signals(void);

private:
	SignalProvider *sp;
	enum ACPILevel requestedState;
	std::unordered_map<enum ACPILevel, Signal *> inputs;
	std::unordered_map<enum ACPILevel, Signal *> outputs;

};

#endif