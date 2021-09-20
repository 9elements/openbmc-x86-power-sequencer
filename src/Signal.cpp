
#include "Signal.hpp"
#include "StateMachine.hpp"
#include <functional>

using namespace std;
using namespace placeholders;

// SignalName returns the instance name
string Signal::SignalName(void)
{
	return this->name;
}

// RegisterPollCallback is used by the data source provider to register a function
// that is able to retrieve the current signal state.
// Not used on signals where the data source provider sets the data asynchronously.
void Signal::RegisterPollCallback(bool func(Signal*))
{
	this->pollSlot.connect(func);
}

Signal::Signal(string name)
{
	this->name = name;
}

// RegisterLevelChangeCallback add the provided function to the list to call
// when the signal level changed. This is being used by output GPIOs, ....
void Signal::RegisterSetLevelCallback(StateMachine &s)
{

}

// RegisterLevelChangeCallback add the provided function to the list to call
// when the signal level changed. This is being used by output GPIOs, ....
void Signal::RegisterLevelChangeCallback(void func (Signal*, bool))
{
	this->levelChangeSlots.connect(func);
}

// RegisterLevelChangeCallback add the provided function to the list to call
// when the signal level changed. This is being used by output GPIOs, ....
boost::signals2::signal<void (Signal*, bool)>& Signal::LevelChangeSignal(void)
{
	return this->levelChangeSlots;
}


// GetLevel returns the internal active state
bool Signal::GetLevel()
{
	return this->active;
}

// SetLevel sets the internal active state.
// It can be called by interrupt handlers.
// It does not invoke the levelChange slots.
void Signal::SetLevel(bool newLevel)
{
	if (this->active != newLevel) {
		this->active = newLevel;
		this->setLevelSlots(this, newLevel);
	}
}

// Poll gathers the state of the signal. Signals that are updated by interrupt handlers
// do nothing in the Poll method. Internally Poll calls SetLevel().
// Poll is the first method called in the schedulers statemachine
void Signal::Poll(void)
{
	this->pollSlot(this);
}

// Apply is called by the scheduler to set the new value after as last part of the
// internal state machine. Apply invokes all levelChange slots.
void Signal::Apply()
{
	this->levelChangeSlots(this, this->active);
}