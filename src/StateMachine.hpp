
#include <vector>
#include "Config.hpp"
#include "Signal.hpp"
#include "GpioInput.hpp"
#include "GpioOutput.hpp"

#include <boost/thread/mutex.hpp>

using namespace std;

class StateMachine {
public:

	StateMachine(
		std::vector<Signal *> 
	);

	// Create statemachine from config
	StateMachine(
		Config&
	);
	
	// ScheduleSignalChange queues a signal change in the current waiting list and
	// notifies the state machine to wake and do some work.
	//
	// This method can by called asynchronously by any thread. It wakes the worker
	// thread. It supposed to be called by GPIO interrupt handlers, logic blocks
	// that calculated a new signal state, FAN or PMBUS fault interrupt handlers, ...
	//
	// FIXME: The same signal might be scheduled multiple times before the worker thread
	// is being invoked. For each scheduled state change the state machine is
	// run once.
	// FIXME: For performance reasons we don't want this.
	void ScheduleSignalChange(Signal*, bool);

	// Run starts the internal state machine.
	//
	// The method is has work to do when at least one signal had been scheduled by
	// a call to ScheduleSignalChange.
	//
	// It calls
	//  ApplySignalLevel()
	//  GatherDependendSignals()
	//  ClearScheduledSignals()
	//  UpdateChangedSignals()
	void Run(bool);
private:

	// ApplySignalLevel applies the new signal state.
	// This might change the output of GPIO pins or enable/disable voltage regulators.
	void ApplySignalLevel(void);

	// Use scheduledSignalLevel to create a new changedSignals list of signals affected
	// by the current change.
	void GatherDependendSignals(void);

	// ClearScheduledSignals removes all signals in scheduledSignalLevel
	void ClearScheduledSignals(void);

	// UpdateChangedSignals invokes the Update method of all changed signals
	// The Update method might fill scheduledSignalLevel again, which will trigger another
	// invokation of Run()
	void UpdateChangedSignals(void);

	// scheduledSignalLevel contains a list of signals that have been changed.
	// pending changes are applied on each invokation of the state machine Run() function
	std::vector<Signal*> scheduledSignalLevel;

	// changedSignals contains a temporary list of signals that have been affected by the last
	// scheduledSignalLevel.
	std::vector<Signal*> changedSignals;

	// List of all signals under state machines control
	std::vector<Signal*> signals;

	// Lock for scheduleSignalLevels
	boost::mutex scheduledLock;

	std::vector<GpioOutput *> gpioOutputs;
	std::vector<GpioInput *> gpioInputs;
};

