
#include "Config.hpp"
#include "IODriver.hpp"
#include "Signal.hpp"

#include <gpiod.hpp>

#include <vector>

using namespace std;

class SignalProvider;
class Signal;

class GpioOutput : SignalReceiver, public OutputDriver
{
  public:
    // Name returns the instance name
    string Name(void);

    // Apply sets the new output state
    void Apply(void);

    void Update(void);

    GpioOutput(struct ConfigOutput* cfg, SignalProvider& prov);
    ~GpioOutput();

  private:
    bool active;
    bool newLevel;
    gpiod::line line;
    gpiod::chip chip;
    Signal* in;
};