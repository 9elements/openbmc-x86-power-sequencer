
#include <vector>
#include <boost/signals2.hpp>
#include <gpiod.hpp>
#include "Config.hpp"
#include "Signal.hpp"

using namespace std;

class GpioInput{
public:
	// Name returns the instance name
	string Name(void);

	// Poll returns the current GPIO state
	bool Poll(Signal&);

	GpioInput(string chipName, string lineName, bool activeLow);
	GpioInput(struct ConfigInput *cfg);

	~GpioInput();

private:
	// Internal state of the signal. Can only be modified by a call to Apply().
	bool active;
	gpiod::line line;
	gpiod::chip chip;
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
     