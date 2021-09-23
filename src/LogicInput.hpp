#ifndef _LOGICINPUT_HPP__
#define _LOGICINPUT_HPP__

using namespace std;
class Signal;
class Logic;

class LogicInput : SignalReceiver {
public:
	// GetLevel is called by Logic when gathering it's new state
	bool GetLevel();

	// Dependency returns the Signal attached to
	Signal* Dependency();

	LogicInput(boost::asio::io_context& io, Signal* in, Logic *par, bool inv, int stable);
	LogicInput(boost::asio::io_context& io,
		SignalProvider& prov,
		struct ConfigLogicInput *cfg,
		Logic *par);
	
	void Update(void);
private:
	int inputStableUsec;
	bool invert;
	bool level;
	Logic *parent;
	Signal* input;
	boost::asio::deadline_timer timer;
};

#endif