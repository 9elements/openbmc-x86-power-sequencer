
#ifndef _NULLOUTPUT_HPP__
#define _NULLOUTPUT_HPP__
#include "Config.hpp"
#include "IODriver.hpp"
#include "Signal.hpp"

using namespace std;

class SignalProvider;
class Signal;

class NullOutput : SignalReceiver, public OutputDriver
{
  public:
    // Name returns the instance name
    string Name(void);

    // Apply sets the new output state
    void Apply(void);

    void Update(void);

    bool GetLevel(void);
    NullOutput(struct ConfigOutput* cfg, SignalProvider& prov);
    ~NullOutput();

  private:
    string name;
    bool active;
    bool newLevel;
    Signal* in;
};
#endif