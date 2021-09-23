#ifndef _SIGNAL_HPP__
#define _SIGNAL_HPP__

#include <vector>
#include <functional>
#include <boost/thread/mutex.hpp>
#include <boost/chrono.hpp>
#include "Validate.hpp"

using namespace std;

class StateMachine;
class SignalProvider;

// The signal receiver reads from the signal.
// The signal keeps track of all receivers.
class SignalReceiver {
public:
	virtual void Update(void) = 0;
};

class Signal : public Validator {
public:
	// SignalName returns the instance name
	string SignalName(void);

	Signal(SignalProvider *parent, string name);

	// GetLevel returns the internal active state
	bool GetLevel();

	// SetLevel sets the internal active state.
	// It can be called by interrupt handlers.
	// It marks the signal as dirty in the parent SignalProvider.
	void SetLevel(bool);

	void Validate(void);

	// Dirty is set when SetLevel changed the level
	bool Dirty(void);

	// ClearDirty clears the dirty bit
	void ClearDirty(void);

	// LastLevelChangeTime returns the time when the signal level was changed
	boost::chrono::steady_clock::time_point LastLevelChangeTime();

	// Receivers returns a list of objects listening to this signal
	std::vector<SignalReceiver *> Receivers(void);

	// AddReceiver adds a signal receiver
	void AddReceiver(SignalReceiver* rec);

	// UpdateReceivers invokes the Update method of all signal receivers
	void UpdateReceivers();
private:

	SignalProvider *parent;
	// Internal state of the signal. Can only be modified by a call to Apply().
	bool active;
	bool dirty;
	string name;

	boost::mutex lock;
	boost::chrono::steady_clock::time_point lastLevelChangeTime;

	std::vector<SignalReceiver *>receivers;
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