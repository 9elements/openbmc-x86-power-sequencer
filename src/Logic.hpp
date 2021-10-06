
#ifndef _LOGIC_HPP__
#define _LOGIC_HPP__

#include "Config.hpp"
#include "Signal.hpp"
#include "SignalProvider.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/system_executor.hpp>

#include <vector>

using namespace std;
class LogicInput;

class Logic : SignalReceiver
{
  public:
    // Name returns the instance name
    string Name(void);

    // Update is called to re-evaluate the output state.
    // On level change the connected signal level is set.
    void Update(void);

    Logic(boost::asio::io_context& io, Signal* signal, string name,
          std::vector<LogicInput*> ands, std::vector<LogicInput*> ors,
          bool outputActiveLow, bool andFirst, bool invertFirst, int delay);

    Logic(boost::asio::io_service& io, SignalProvider& prov,
          struct ConfigLogic* cfg);
    ~Logic();

  private:
    // GetLevelAndInputs retuns the logical 'and' of all AND inputs (ignoring
    // andThenOr)
    bool GetLevelAndInputs(void);
    // GetLevelOrInputs retuns the logical 'or' of all OR inputs (ignoring
    // andThenOr)
    bool GetLevelOrInputs(void);
    void TimerHandler(const boost::system::error_code& error,
                      const bool result);

    std::vector<LogicInput*> andInputs;
    std::vector<LogicInput*> orInputs;

    bool outputActiveLow;
    bool andThenOr;
    bool invertFirstGate;
    int delayOutputUsec;
    string name;
    std::time_t outputLastChanged;
    bool lastValue;
    boost::asio::deadline_timer timer;
    Signal* signal;
};

#endif