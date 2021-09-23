

#ifndef _NULLINPUT_HPP__
#define _NULLINPUT_HPP__
#include <boost/asio.hpp>
#include "Config.hpp"

#include "Signal.hpp"

using namespace std;

class Signal;
class Signalprovider;

class NullInput {
public:
	// Name returns the instance name
	string Name(void);

	NullInput(boost::asio::io_context& io, struct ConfigInput* cfg, SignalProvider& prov);

	~NullInput();
	void SetLevel(bool level);

private:
	string name;
	bool active;
	Signal *out;
};
#endif