
#include "Signal.hpp"
#include "StateMachine.hpp"

using namespace std;
using namespace placeholders;

// SignalName returns the instance name
string Signal::SignalName(void)
{
	return this->name;
}

// RegisterPollCallback is used by the data source provider to register a function
// that is able to retrieve the current signal state.
// It does nothing on signals where the data source provider sets the data asynchronously.
// A pollSlot without any listeners is an error.
void Signal::RegisterPollCallback(bool func(Signal*))
{
	this->pollSlot.connect(func);
}

Signal::Signal(string name)
{
	this->name = name;
	this->lastLevelChangeTime = boost::chrono::steady_clock::now();
}

// RegisterLevelChangeCallback add the provided function to the list to call
// when the signal level changed. This is being used by output GPIOs, ....
void Signal::RegisterSetLevelCallback(void func (Signal*, bool))
{
	this->setLevelSlots.connect(func);
}

void Signal::RegisterSetLevelCallback(std::function< void(Signal*, bool) >& lambda)
{
	this->setLevelSlots.connect(lambda);
}

// RegisterLevelChangeCallback add the provided function to the list to call
// when the signal level changed. This is being used by output GPIOs, ....
void Signal::RegisterLevelChangeCallback(void func (Signal*, bool))
{
	this->levelChangeSlots.connect(func);
}

// SetLevelSignal provides access to the signal that is emitted when SetLevel
// is called and the level had changed.
boost::signals2::signal<void (Signal*, bool)>& Signal::SetLevelSignal(void)
{
	return this->setLevelSlots;
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
		this->lastLevelChangeTime = boost::chrono::steady_clock::now();
		this->setLevelSlots(this, newLevel);
	}
}

boost::chrono::steady_clock::time_point Signal::LastLevelChangeTime()
{
	return this->lastLevelChangeTime;
}

// Validate makes sure the signal is ready for use.
void Signal::Validate(void)
{
	if (this->levelChangeSlots.num_slots() == 0) {
		throw std::runtime_error("no one listens to signal " + this->name);
	}
	if (this->pollSlot.num_slots() == 0) {
		throw std::runtime_error("no one sends data to signal " + this->name);
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