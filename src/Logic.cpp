#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>

#include "Logic.hpp"
#include "LogicInput.hpp"

using namespace std;

// Name returns the instance name
string Logic::Name(void)
{
	return this->name;
}

bool Logic::GetLevelOrInputs(void)
{
	bool intermediate = false;

	for (auto it : this->orInputs) {
		if (it->GetLevel()) {
			intermediate = true;
			break;
		}
	}

	return intermediate;
}

bool Logic::GetLevelAndInputs(void)
{
	bool intermediate = true;

	for (auto it : this->andInputs) {
		if (!it->GetLevel()) {
			intermediate = false;
			break;
		}
	}

	return intermediate;
}

// Update is called by Unit to determine the current Logic output state.
// On level change the signal is updated.
void Logic::Update(void)
{
	bool result;

	if (this->andThenOr) {
		result = this->GetLevelAndInputs();

		if (this->invertFirstGate)
			result = !result;

		result |= this->GetLevelOrInputs();

	} else {
		result = this->GetLevelOrInputs();

		if (this->invertFirstGate)
			result = !result;

		result &= this->GetLevelAndInputs();
	}

	if (this->lastValue != result) {
		if (this->delayOutputUsec > 0) {
			// FIXME: Does it reset the timer?
			this->timer.expires_from_now(boost::posix_time::microseconds(this->delayOutputUsec));
			this->timer.async_wait(boost::bind(&Logic::TimerHandler, this, boost::asio::placeholders::error, result));
		} else {
			this->signal->SetLevel(result);
		}

		this->lastValue = result;
	}
}

void Logic::TimerHandler(const boost::system::error_code& error, const bool result)
{
	if (!error) {
		this->signal->SetLevel(result);
	}
}

Logic::Logic(boost::asio::io_context& io,
	Signal* signal,
	string name,
	std::vector<LogicInput *> ands,
	std::vector<LogicInput *> ors,
	bool outputActiveLow,
	bool andFirst,
	bool invertFirst,
	int delay) :
	timer(io)
{
	this->signal = signal;
	this->name = name;
	this->andInputs = ands;
	this->orInputs = ors;

	this->andThenOr = andFirst;
	this->invertFirstGate = invertFirst;
	this->delayOutputUsec = delay;
}

Logic::Logic(boost::asio::io_context& io,
	SignalProvider& prov,
	struct ConfigLogic *cfg) :
	timer (io)
{
	for (auto it: cfg->AndSignalInputs) {
	  	this->andInputs.push_back(new LogicInput(io, prov, &it, this));
	}

	for (auto it: cfg->OrSignalInputs) {
	  	this->orInputs.push_back(new LogicInput(io, prov, &it, this));
	}

	this->signal = prov.FindOrAdd(cfg->Out.SignalName);
	this->name = cfg->Name;
	this->andThenOr = cfg->AndThenOr;
	this->invertFirstGate = cfg->InvertFirstGate;
	this->delayOutputUsec = cfg->DelayOutputUsec;
	this->outputActiveLow = cfg->Out.ActiveLow;
}

Logic::~Logic()
{
	for (auto it: this->andInputs) {
		delete it;
	}

	for (auto it: this->orInputs) {
		delete it;
	}

}