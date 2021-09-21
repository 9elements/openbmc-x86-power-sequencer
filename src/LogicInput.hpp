
using namespace std;
class Signal;
class Logic;

class LogicInput {
public:
	// GetLevel is called by Logic when gathering it's new state
	bool GetLevel();

	// Dependency returns the Signal attached to
	Signal* Dependency();

	LogicInput(Signal* in, Logic *par, bool inv, int stable);
private:
	int inputStableUsec;
	bool invert;
	Logic *parent;
	Signal* input;
};

