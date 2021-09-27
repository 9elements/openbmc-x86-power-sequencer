
#include <vector>
#include "Config.hpp"
#include "SignalProvider.hpp"
#include "Signal.hpp"
#include "GpioInput.hpp"
#include "GpioOutput.hpp"
#include "NullInput.hpp"
#include "NullOutput.hpp"
#include "Logic.hpp"
#include "OutputDriver.hpp"

#include <boost/thread/mutex.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>

using namespace std;

class StateMachineTester;

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

protected:
	std::vector<NullOutput *> GetNullOutputs(void);
	std::vector<NullInput *> GetNullInputs(void);

	// ApplyOutputSignalLevel applies the new signal state.
	// This will change the output of GPIO pins or enable/disable voltage regulators.
	void ApplyOutputSignalLevel(void);

private:
	bool running;


	// Lock for scheduleSignalLevels
	boost::mutex scheduledLock;

	std::vector<OutputDriver *> outputDrivers;
	std::vector<NullOutput *> nullOutputs;
	std::vector<NullInput *> nullInputs;

	std::vector<GpioOutput *> gpioOutputs;
	std::vector<GpioInput *> gpioInputs;
	std::vector<Logic *> logic;

	// The ASIO boost service for timers
	boost::asio::io_context io;

	SignalProvider *sp;
	friend class StateMachineTester;
};

class StateMachineTester {
public:
	std::vector<NullOutput *> GetNullOutputs(void)
	{
		return sm->GetNullOutputs();
	}
	std::vector<NullInput *> GetNullInputs(void) {
		return sm->GetNullInputs();
	}
	StateMachineTester(StateMachine *sm) : sm {sm}
	{};

	void ApplyOutputSignalLevel(void)
	{
		sm->ApplyOutputSignalLevel();
	}

private:
	StateMachine *sm;
};
