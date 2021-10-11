

#ifndef _NULLINPUT_HPP__
#define _NULLINPUT_HPP__
#include "Config.hpp"
#include "IODriver.hpp"
#include "Signal.hpp"

#include <boost/asio.hpp>

using namespace std;

class Signal;
class Signalprovider;

class NullInput : public SignalDriver
{
  public:
    // Name returns the instance name
    string Name(void);

    NullInput(boost::asio::io_context& io, struct ConfigInput* cfg,
              SignalProvider& prov);

    ~NullInput();
    void SetLevel(bool level);

    vector<Signal*> Signals(void);

  private:
    string name;
    bool active;
    Signal* out;
};
#endif