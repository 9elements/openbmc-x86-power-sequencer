
#include <vector>
#include "Config.hpp"
#include "SignalProvider.hpp"
#include "Signal.hpp"
#include "GpioInput.hpp"
#include "GpioOutput.hpp"
#include "Logic.hpp"
#include "OutputDriver.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

using namespace std;

class StateMachine {
public:
	// Create statemachine from config
	StateMachine(
		Config&,
		SignalProvider&
	);

	// Run starts the internal state machine.
	//
	// The method is has work to do when at least one signal had been scheduled by
	// a call to ScheduleSignalChange.
	//
	// It calls

	void Run(void);

	void EvaluateState(void);

	// OnDirtySet is called when a signal has the dirty bit set
	void OnDirtySet(void);
private:
	bool running;

	// ApplyOutputSignalLevel applies the new signal state.
	// This will change the output of GPIO pins or enable/disable voltage regulators.
	void ApplyOutputSignalLevel(void);

	// Lock for scheduleSignalLevels
	boost::mutex scheduledLock;

	std::vector<OutputDriver *> outputDrivers;
	std::vector<GpioOutput *> gpioOutputs;
	std::vector<GpioInput *> gpioInputs;
	std::vector<Logic *> logic;

	// The ASIO boost service for timers
	boost::asio::io_context io;

	SignalProvider *sp;
};

