#ifndef _SIGNAL_HPP__
#define _SIGNAL_HPP__

#include <vector>
#include <functional>
#include <boost/signals2.hpp>

using namespace std;

class StateMachine;

class Signal {
public:
	// SignalName returns the instance name
	string SignalName(void);

	// RegisterPollCallback is used by the data source provider to register a function
	// that is able to retrieve the current signal state.
	// Not used on signals where the data source provider sets the data asynchronously.
	void RegisterPollCallback(bool (Signal*));

	Signal(string name);

	// RegisterLevelChangeCallback add the provided function to the list to call
	// when the signal level changed. This is being used by output driver.
	void RegisterLevelChangeCallback(void func (Signal*, bool));

	boost::signals2::signal<void (Signal*, bool)>& LevelChangeSignal(void);

	// GetLevel returns the internal active state
	bool GetLevel();

	// SetLevel sets the internal active state.
	// It can be called by interrupt handlers.
	// It does not invoke the levelChange slots.
	void SetLevel(bool);
protected:

	// RegisterSetLevelCallback add the provided function to the list to call
	// when the signal level is set by the input driver.
	void RegisterSetLevelCallback(void func (Signal*, bool));
	void RegisterSetLevelCallback(std::function< void(Signal*, bool) >& lambda);

	// Poll gathers the state of the signal. Signals that are updated by interrupt handlers
	// do nothing in the Poll method. Internally Poll calls SetLevel().
	// Poll is the first method called in the schedulers statemachine
	void Poll(void);

	// Apply is called by the scheduler to set the new value after as last part of the
	// internal state machine. Apply invokes all levelChange slots.
	void Apply();
private:
	// Internal state of the signal. Can only be modified by a call to Apply().
	bool active;
	string name;
	boost::signals2::signal<void (Signal*, bool)> levelChangeSlots;
	boost::signals2::signal<bool (Signal*)> pollSlot;
	boost::signals2::signal<void (Signal*, bool)> setLevelSlots;

	friend StateMachine;
};

//
// Signal flow
//
// GPIO Interrupt
// -> Kernel interrupt
// -> User space interrupt
// -> GPIOInput::Interrupt(bool newLevel)
// -> Call Signal::SetLevel(bool newLevel) // returns if oldLevel == newLevel
//  -> Calls StateMachine::ScheduleSignalChange()
//   -> StateMachine::Run()
//    -> Signal::Poll() // does nothing as signal is interrupt driven
//   -> StateMachine::SomethingElse()
//    -> Signal::Apply() // Invokes all slots
//     -> GPIOOutput::Apply(Signal&, bool newLevel)
//      -> gpiod_setoutput(x, newLevel)
     
#endif