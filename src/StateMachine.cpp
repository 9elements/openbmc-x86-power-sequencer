
#include "StateMachine.hpp" 
#include <boost/thread/lock_guard.hpp>
#include <functional>
using namespace std;
using namespace placeholders;

StateMachine::StateMachine(
	std::vector<Signal* > signals
)
{
	this->signals = signals;
}

// Create statemachine from config
StateMachine::StateMachine(
	Config &cfg,
	SignalProvider& prov
)
{

  prov.RegisterDirtyBitEvent([&](void) { this->OnDirtySet(); });

  for (int i = 0; i < cfg.Logic.size(); i++) {
      this->logic.push_back(new Logic(this->io, prov, &cfg.Logic[i]));
      std::cout << "pushing logic " << cfg.Logic[i].Name << " to list "  << std::endl;
  }
  for (int i = 0; i < cfg.Inputs.size(); i++) {
    if (cfg.Inputs[i].InputType == INPUT_TYPE_GPIO) {

      this->gpioInputs.push_back(new GpioInput(this->io, &cfg.Inputs[i], prov));

      std::cout << "pushing gpio input " << cfg.Inputs[i].SignalName << " to list "  << std::endl;
    }
  }

  for (int i = 0; i < cfg.Outputs.size(); i++) {
    if (cfg.Outputs[i].OutputType == OUTPUT_TYPE_GPIO) {
      this->gpioOutputs.push_back(new GpioOutput(&cfg.Outputs[i], prov));

      std::cout << "pushing gpio output " << cfg.Outputs[i].SignalName << " to list "  << std::endl;
    }
  }

  this->sp = &prov;

  /*
  for (int i = 0; i < vec.size(); i++) {
    vec[i]->Validate();
  }*/

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
void StateMachine::ScheduleSignalChange(Signal* signal, bool newLevel)
{
  boost::lock_guard<boost::mutex> lock(this->scheduledLock);
  std::cout << "ScheduleSignalChange event from " << signal->SignalName() << ", new level " << newLevel << std::endl;
 // this->scheduledSignalLevel[signal] = newLevel;
}

void StateMachine::OnDirtySet(void)
{
  boost::lock_guard<boost::mutex> lock(this->scheduledLock);

  std::cout << "StateMachine::OnDirtySet" << std::endl;
 // this->scheduledSignalLevel[signal] = newLevel;
}

// Run starts the internal state machine.
//
// The method is has work to do when at least one signal had been scheduled by
// a call to ScheduleSignalChange.
//
void StateMachine::Run(bool)
{
  boost::lock_guard<boost::mutex> lock(this->scheduledLock);

  std::vector<Signal *> signals;
  
  signals = this->sp->DirtySignals();
  while (signals.size() > 0) {

    for (auto sig: signals) {
      sig->UpdateReceivers();
    }
    // The Update call might have added new dirty signals.
    // FIXME: Add timeout and loop detection.
    signals = this->sp->DirtySignals();
  }

  // State is stable
  // TODO: Dump state
  // TODO: set outputs
}