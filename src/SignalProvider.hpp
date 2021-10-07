#ifndef _SIGNALPROVIDER_HPP__
#define _SIGNALPROVIDER_HPP__

#include "Config.hpp"
#include "Signal.hpp"

#include <fstream>
#include <map>
#include <vector>

using namespace std;

class SignalDriver;
// The SignalProvider class owns all signals
class SignalProvider
{
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
    void Validate(std::vector<SignalDriver*> drvs);

    // DirtySignals provides a list of signals having the "dirty" bit set
    std::vector<Signal*> DirtySignals();

    // ClearDirty removes the dirty bit of all signals and clears the list
    void ClearDirty(void);

    // SetDirty adds the signal to the dirty listt
    void SetDirty(Signal*);

    // SetDirtyBitEvent
    void SetDirtyBitEvent(std::function<void(void)> const& lamda);

  private:
    // Add a new signal
    Signal* Add(string name);

    std::ofstream outfile;
    string dumpFolder;

    std::map<std::string, Signal*> signals;
    std::vector<Signal*> dirty;

	std::function< void(void) > dirtyBitSignal;

    std::vector<std::string> floatingSignals;
    std::string signalDumpFolder;
};

#endif