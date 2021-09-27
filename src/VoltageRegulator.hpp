
#include <vector>
#include "Config.hpp"
#include "Signal.hpp"
#include "IODriver.hpp"

using namespace std;

class SignalProvider;
class Signal;

class VoltageRegulator : SignalReceiver, public OutputDriver, public InputDriver {
public:
	// Name returns the instance name
	string Name(void);

	// Apply sets the new output state
	void Apply(void);

	// Update fetches the current state from connected signals
	void Update(void);

	// Signals returns the list of signals that are feed with data
	std::vector<Signal *> Signals(void);

	VoltageRegulator(struct ConfigRegulator *cfg, SignalProvider& prov);
	~VoltageRegulator();

private:
	string name;

	bool active;
	bool newLevel;
	bool hasfault;
	bool alwaysOn;

	Signal *in;
	Signal *enabled;
	Signal *fault;
	Signal *powergood;
};