
#include <vector>
#include <boost/signals2.hpp>
#include <boost/asio.hpp>

#include <gpiod.hpp>
#include "Config.hpp"
#include "Signal.hpp"
#include "IODriver.hpp"

using namespace std;

class Signal;
class Signalprovider;

class GpioInput : public InputDriver {
public:
	// Name returns the instance name
	string Name(void);

	void Poll(const boost::system::error_code& e);

	GpioInput(boost::asio::io_context& io, struct ConfigInput *cfg, SignalProvider& prov);

	~GpioInput();
	std::vector<Signal *> Signals(void);

private:
	bool active;
	boost::asio::steady_timer timer;
	gpiod::line line;
	gpiod::chip chip;
	Signal *out;
};