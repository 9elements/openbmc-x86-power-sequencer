
#include "StateMachine.hpp" 
#include <boost/thread/lock_guard.hpp>

using namespace std;

StateMachine::StateMachine(
	std::vector<Signal* > signals
)
{
	this->signals = signals;
}

// Create statemachine from config
StateMachine::StateMachine(
	Config cfg
)
{
  for (int i = 0; i < cfg.Inputs.size(); i++) {
    if (cfg.Inputs[i].InputType == INPUT_TYPE_GPIO) {
      this->gpioInputs.push_back(new GpioInput(&cfg.Inputs[i]));
    }
  }
  for (int i = 0; i < cfg.Outputs.size(); i++) {
    if (cfg.Outputs[i].OutputType == OUTPUT_TYPE_GPIO) {
      this->gpioOutputs.push_back(new GpioOutput(&cfg.Outputs[i]));
    }
  }
}

// ScheduleSignalChange queues a signal change in the current waiting list and
// notifies the state machine to wake and do some work.
//
// This method can by called asynchronously by any thread. It wakes the worker
// thread. It supposed to be called by GPIO interrupt handlers, logic blocks
// that calculated a new signal state, FAN or PMBUS fault interrupt handlers, ...
//
// The same signal might be scheduled multiple times before the worker thread
// is being invoked. For each scheduled state change the state machine is
// run once.
void StateMachine::ScheduleSignalChange(Signal& signal, bool newLevel)
{
  boost::lock_guard<boost::mutex> lock(this->scheduledLock);
 // this->scheduledSignalLevel[signal] = newLevel;
}

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
void StateMachine::Run(bool)
{
  boost::lock_guard<boost::mutex> lock(this->scheduledLock);

}

// ApplySignalLevel applies the new signal state.
// This might change the output of GPIO pins or enable/disable voltage regulators.
void StateMachine::ApplySignalLevel(void)
{
	
}

// Use scheduledSignalLevel to create a new changedSignals list of signals affected
// by the current change.
void StateMachine::GatherDependendSignals(void)
{
	
}

// ClearScheduledSignals removes all signals in scheduledSignalLevel
void StateMachine::ClearScheduledSignals(void)
{
	
}

// UpdateChangedSignals invokes the Update method of all changed signals
// The Update method might fill scheduledSignalLevel again, which will trigger another
// invokation of Run()
void StateMachine::UpdateChangedSignals(void)
{
	
}
