#ifndef _SIGNALPROVIDER_HPP__
#define _SIGNALPROVIDER_HPP__

#include <vector>
#include "Validate.hpp"
#include "Signal.hpp"

using namespace std;

// The SignalProvider class owns all signals
class SignalProvider : public Validator {
public:
	// Add a new signal
	Signal* Add(string name);

	// Find returns a signal by name. NULL if not found.
	Signal* Find(string name);

	// Find returns a signal by name. If not found a new signal is added
	Signal* FindOrAdd(string name);

	SignalProvider();
	~SignalProvider();

	void Validate(void);
private:
	std::vector<Signal *> signals;
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