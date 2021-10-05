#ifndef _SIGNALPROVIDER_HPP__
#define _SIGNALPROVIDER_HPP__

#include <vector>
#include <fstream>
#include <boost/signals2.hpp>

#include "Signal.hpp"
#include "Config.hpp"

using namespace std;

class InputDriver;
// The SignalProvider class owns all signals
class SignalProvider {
public:
	// Find returns a signal by name. nullptr if not found.
	Signal* Find(string name);

	// FindOrAdd returns a signal by name. If not found a new signal is added
	Signal* FindOrAdd(string name);

	// DumpSignals writes the signal state to the folder
	void DumpSignals(void);

	SignalProvider(Config& cfg, string dumpFolder = "");
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
	// Add a new signal
	Signal* Add(string name);

	std::ofstream outfile;
	string dumpFolder;
	
	boost::mutex lock;
	std::vector<Signal *> signals;
	std::vector<Signal *> dirty;
	boost::signals2::signal<void (void)> dirtyBitSignal;
	std::vector<std::string> floatingSignals;
	std::string signalDumpFolder;
};

#endif