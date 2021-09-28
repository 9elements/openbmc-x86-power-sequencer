
#include <vector>
#include <boost/filesystem.hpp>

#include "Config.hpp"
#include "Signal.hpp"
#include "IODriver.hpp"

using namespace std;

class SignalProvider;
class Signal;

enum RegulatorState {
	ENABLED = 0,
	DISABLED = 1,
	UNKNOWN = 2,
};

enum RegulatorStatus {
	OFF = 0,
	ON = 1,
	ERROR = 2,
	FAST = 3,
	NORMAL = 4,
	IDLE = 5,
	STANDBY = 6,
};

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
	// SysFsRootDirByName returns "/sys/class/regulator/..." path of matching regulator
	std::string SysFsRootDirByName(std::string name);

	// ReadStatus parses /sys/class/regulator/.../status 
	enum RegulatorStatus ReadStatus(void); 

	// ReadState parses /sys/class/regulator/.../state
	enum RegulatorState ReadState(void); 

	// ReadStates updates the signals from sysfs attributes
	void ReadStates(void);

	// SetState writes to /sys/class/regulator/.../state
	void SetState(const enum RegulatorState state);

	string name;
	boost::filesystem::path sysfsRoot;

	bool active;
	bool newLevel;
	bool hasfault;
	bool alwaysOn;

	Signal *in;
	Signal *enabled;
	Signal *fault;
	Signal *powergood;
};