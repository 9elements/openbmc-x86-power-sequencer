#ifndef _SIGNAL_HPP__
#define _SIGNAL_HPP__

#include "Validate.hpp"

#include <boost/thread/mutex.hpp>

#include <chrono>
#include <functional>
#include <vector>

using namespace std;
using namespace std::chrono;

class StateMachine;
class SignalProvider;

// The signal receiver reads the current level from the Signal.
// The signal keeps track of all receivers.
class SignalReceiver
{
  public:
    virtual void Update(void) = 0;
};

class Signal
{
  public:
    // SignalName returns the instance name
    string SignalName(void);

    Signal(SignalProvider* parent, string name);

    // GetLevel returns the internal active state
    bool GetLevel();

    // SetLevel sets the internal active state.
    // It can be called by interrupt handlers.
    // It marks the signal as dirty in the parent SignalProvider.
    void SetLevel(bool);

    void Validate(std::vector<std::string>&);

    // Dirty is set when SetLevel changed the level
    bool Dirty(void);

    // ClearDirty clears the dirty bit
    void ClearDirty(void);

    // LastLevelChangeTime returns the time when the signal level was changed
    steady_clock::time_point LastLevelChangeTime();

    // Receivers returns a list of objects listening to this signal
    std::vector<SignalReceiver*> Receivers(void);

    // AddReceiver adds a signal receiver
    void AddReceiver(SignalReceiver* rec);

    // UpdateReceivers invokes the Update method of all signal receivers
    void UpdateReceivers();

  private:
    SignalProvider* parent;
    // Internal state of the signal. Can only be modified by a call to Apply().
    bool active;
    bool dirty;
    string name;

    boost::mutex lock;
    steady_clock::time_point lastLevelChangeTime;

    std::vector<SignalReceiver*> receivers;
    friend StateMachine;
};

#endif