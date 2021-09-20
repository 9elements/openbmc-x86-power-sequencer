
#include <vector>
#include <boost/signals2.hpp>
#include <gpiod.hpp>
#include "Config.hpp"

using namespace std;

class GpioOutput {
public:
	// Name returns the instance name
	string Name(void);

	// Apply sets the new output state
	void Apply(bool newLevel);

	GpioOutput(string chipName, string lineName, bool activeLow);
	GpioOutput(struct ConfigOutput *cfg);
	~GpioOutput();

private:
	bool active;
	gpiod::line line;
	gpiod::chip chip;
};