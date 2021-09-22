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

	// DumpSignals writes the signal state to the folder
	void DumpSignals(string path);

	SignalProvider();
	~SignalProvider();

	// Validate iterates over all signals and calls their validate method
	void Validate(void);

	// DirtySignals provides a list of signals having the "dirty" bit set
	std::vector<Signal *> DirtySignals();

	// ClearDirty removes the dirty bit
	void ClearDirty(void);

	// SetDirty sets the dirty bit
	void SetDirty(Signal *);

private:
	boost::mutex lock;
	std::vector<Signal *> signals;
	std::vector<Signal *> dirty;
};

#endif