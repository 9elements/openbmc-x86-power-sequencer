
#include "StateMachine.hpp" 
#include <boost/thread/lock_guard.hpp>
#include <functional>
using namespace std;
using namespace placeholders;

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
      GpioInput *g = new GpioInput(this->io, &cfg.Inputs[i], prov);
      this->gpioInputs.push_back(g);
      std::cout << "pushing gpio input " << cfg.Inputs[i].SignalName << " to list "  << std::endl;
      this->inputDrivers.push_back(g);
    } else if (cfg.Inputs[i].InputType == INPUT_TYPE_NULL) {
      NullInput *n = new NullInput(this->io, &cfg.Inputs[i], prov);
      this->nullInputs.push_back(n);
      std::cout << "pushing null input " << cfg.Inputs[i].SignalName << " to list "  << std::endl;
      this->inputDrivers.push_back(n);
    }
  }

  for (int i = 0; i < cfg.Outputs.size(); i++) {
    if (cfg.Outputs[i].OutputType == OUTPUT_TYPE_GPIO) {
      GpioOutput *g = new GpioOutput(&cfg.Outputs[i], prov);
      this->gpioOutputs.push_back(g);
      this->outputDrivers.push_back(g);
      std::cout << "pushing gpio output " << cfg.Outputs[i].SignalName << " to list "  << std::endl;
    } else if (cfg.Outputs[i].OutputType == OUTPUT_TYPE_NULL) {
      NullOutput *n = new NullOutput( &cfg.Outputs[i], prov);
      this->nullOutputs.push_back(n);
      std::cout << "pushing null output " << cfg.Inputs[i].SignalName << " to list "  << std::endl;
      this->outputDrivers.push_back(n);
    }
  }

  this->Validate();
  //
  this->sp = &prov;
  this->running = false;

}

// Validates checks if the current config is sane
void StateMachine::Validate(void)
{
  this->sp->Validate(this->inputDrivers);
}

// ApplyOutputSignalLevel writes signal state to outputs
void StateMachine::ApplyOutputSignalLevel(void)
{
 for (auto it: this->outputDrivers) {
	  it->Apply();
  }
}

// OnDirtySet is invoked when a signal dirty bit is set
void StateMachine::OnDirtySet(void)
{
  boost::lock_guard<boost::mutex> lock(this->scheduledLock);
  if (!this->running) {
    this->io.post([&](){this->EvaluateState();});
  }

  std::cout << "StateMachine::OnDirtySet" << std::endl;
 // this->scheduledSignalLevel[signal] = newLevel;
}

// EvaluateState runs until no more signals change
// FIXME: An input GPIO might set the dirty bit while this code
// runs. Add an timer for fixed runtime? block inputs while this runs?
void StateMachine::EvaluateState(void)
{
  {
    boost::lock_guard<boost::mutex> lock(this->scheduledLock);
    this->running = true;
  }

  std::vector<Signal *> signals;

  signals = this->sp->DirtySignals();
  while (signals.size() > 0) {

    for (auto sig: signals) {
      sig->UpdateReceivers();
    }
    // The Update call might have added new dirty signals.
    // FIXME: Add timeout and loop detection.
    signals = this->sp->DirtySignals();
    // Fixme: Add DirtySignalsAndClear to prevent race condition
    this->sp->ClearDirty();
  }

  // State is stable
  // TODO: Dump state

  this->ApplyOutputSignalLevel();
  {
    boost::lock_guard<boost::mutex> lock(this->scheduledLock);
    this->running = false;
  }
}

// Run does work on the io_queue.
void StateMachine::Run(void)
{
	while (1)
		this->io.run();
}

std::vector<NullOutput *> StateMachine::GetNullOutputs(void)
{
	return this->nullOutputs;
}

std::vector<NullInput *> StateMachine::GetNullInputs(void)
{
	return this->nullInputs;
}