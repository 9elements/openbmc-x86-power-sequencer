
#include "Signal.hpp"
#include "StateMachine.hpp"
#include "SignalProvider.hpp"

#include <boost/thread/lock_guard.hpp>

using namespace std;
using namespace placeholders;

// SignalName returns the instance name
string Signal::SignalName(void)
{
	return this->name;
}

Signal::Signal(SignalProvider *parent, string name)
{
       this->parent = parent;
       this->name = name;
       this->lastLevelChangeTime = boost::chrono::steady_clock::now();
}

// GetLevel returns the internal active state
bool Signal::GetLevel()
{
	return this->active;
}

// SetLevel sets the internal active state.
// It can be called by interrupt handlers.
// It does not invoke the levelChange slots.
void Signal::SetLevel(bool newLevel)
{
	if (this->active != newLevel) {
		boost::lock_guard<boost::mutex> guard(this->lock);
		this->active = newLevel;
		this->lastLevelChangeTime = boost::chrono::steady_clock::now();
		this->parent->SetDirty(this);
	}
}

boost::chrono::steady_clock::time_point Signal::LastLevelChangeTime()
{
	return this->lastLevelChangeTime;
}

void Signal::ClearDirty(void)
{
  boost::lock_guard<boost::mutex> guard(this->lock);
  this->dirty = false;
}

bool Signal::Dirty(void)
{
  boost::lock_guard<boost::mutex> guard(this->lock);
  return this->dirty;
}

// Validate makes sure the signal is ready for use.
void Signal::Validate(void)
{
	if (this->receivers.size() == 0) {
		throw std::runtime_error("no one listens to signal " + this->name);
	}
}

// Receivers returns a list of objects listening to this signal
std::vector<SignalReceiver *> Signal::Receivers(void)
{
	return this->receivers;
}

// AddReceiver adds a signal receiver
void Signal::AddReceiver(SignalReceiver& rec)
{
	this->receivers.push_back(&rec);
}
