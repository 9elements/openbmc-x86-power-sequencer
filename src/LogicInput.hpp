
using namespace std;
class Signal;
class Logic;

class LogicInput {
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

private:
	int inputStableUsec;
	bool invert;
	bool level;
	Logic *parent;
	Signal* input;
	boost::asio::deadline_timer timer;
};

