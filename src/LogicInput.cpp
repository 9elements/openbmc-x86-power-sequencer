#include <iostream>
#include <boost/chrono.hpp>

#include "Logic.hpp"
#include "LogicInput.hpp"

using namespace std;

// GetLevel is called by Logic when gathering it's new state
bool LogicInput::GetLevel()
{
	if (this->inputStableUsec == 0) {
		this->level = this->input->GetLevel();

	} else {
		boost::chrono::nanoseconds ns = boost::chrono::steady_clock::now() - this->input->LastLevelChangeTime();

		if (ns.count() >= this->inputStableUsec * 1000) {
			this->level = this->input->GetLevel();
		} else {
			// Invoke the parent's Update method. It will update the signal's state if necessary.
			// It also invokes this function, which, when necessary, will schedule another timer.
			this->timer.expires_from_now(boost::posix_time::microseconds(this->inputStableUsec - ns.count() / 1000) );
			this->timer.async_wait([&] ( const boost::system::error_code& err ) {
				if (!err) {
					this->parent->Update();
				}
			});
		}
	}
	
	return this->level;
}

// Dependency returns the Unit attached to
Signal* LogicInput::Dependency()
{
	return this->input;
}

// Update is called whenever a signal changed
void LogicInput::Update(void)
{
	if (this->level != this->GetLevel()) {
		// Notify the parent if input has changed.
		this->parent->Update();
	}
}

LogicInput::LogicInput(boost::asio::io_context& io, Signal* in, Logic* par, bool inv, int stable) :
	timer(io)
{
	this->input = in;
	this->parent = par;
	this->invert = inv;
	this->inputStableUsec = stable;
	this->level = false;
}

LogicInput::LogicInput(boost::asio::io_context& io,
		SignalProvider& prov,
		struct ConfigLogicInput *cfg,
		Logic *par) :
		timer(io)
{
	Signal *sig = prov.FindOrAdd(cfg->SignalName);
	sig->AddReceiver(this);

	this->input = sig;
	this->parent = par;
	this->invert = cfg->Invert;
	this->inputStableUsec = cfg->InputStableUsec;
	this->level = false;
}