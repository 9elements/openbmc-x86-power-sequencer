#ifndef _SIGNALPROVIDER_HPP__
#define _SIGNALPROVIDER_HPP__

//#include "Config.hpp"

#include <fstream>
#include <functional>
#include <map>
#include <vector>

using namespace std;

class SignalDriver;
class Signal;
struct Config;

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
    void Validate(vector<SignalDriver*> drvs);

    // GetDirtySignalsAndClearList provides a pointer to vector of signals
    // that had the "dirty" bit set. The invokation unsets the dirty bit,
    // clears the internal list and returns a copy to it that can be used until
    // this function is called again.
    vector<Signal*>* GetDirtySignalsAndClearList();

    // ClearDirty removes the dirty bit of all signals and clears the list
    void ClearDirty(void);

    // SetDirty adds the signal to the dirty listt
    void SetDirty(Signal*);

    // SetDirtyBitEvent
    void SetDirtyBitEvent(function<void(void)> const& lamda);

  private:
    // Add a new signal
    Signal* Add(string name);

    ofstream outfile;
    string dumpFolder;

    map<string, Signal*> signals;
    // Double buffered dirty vector
    bool dirtyAIsActive;
    vector<Signal*> dirtyA;
    vector<Signal*> dirtyB;

    function<void(void)> dirtyBitSignal;

    vector<string> floatingSignals;
    string signalDumpFolder;
};

#endif