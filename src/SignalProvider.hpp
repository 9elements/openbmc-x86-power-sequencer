#ifndef _SIGNALPROVIDER_HPP__
#define _SIGNALPROVIDER_HPP__

#include <vector>
#include <boost/signals2.hpp>

#include "Signal.hpp"

using namespace std;

class InputDriver;
// The SignalProvider class owns all signals
class SignalProvider {
public:
	// Add a new signal
	Signal* Add(string name);

	// Find returns a signal by name. NULL if not found.
	Signal* Find(string name);

	// FindOrAdd returns a signal by name. If not found a new signal is added
	Signal* FindOrAdd(string name);

	// DumpSignals writes the signal state to the folder
	void DumpSignals(string path);

	SignalProvider();
	~SignalProvider();

	// Validate iterates over all signals and calls their validate method
	void Validate(std::vector<InputDriver *> drvs);

	// DirtySignals provides a list of signals having the "dirty" bit set
	std::vector<Signal *> DirtySignals();

	// ClearDirty removes the dirty bit of all signals and clears the list
	void ClearDirty(void);

	// SetDirty adds the signal to the dirty listt
	void SetDirty(Signal *);

	// RegisterDirtyBitEvent
	void RegisterDirtyBitEvent(std::function<void (void)> const& lamda);
private:
	boost::mutex lock;
	std::vector<Signal *> signals;
	std::vector<Signal *> dirty;
	boost::signals2::signal<void (void)> dirtyBitSignal;
};

#endif