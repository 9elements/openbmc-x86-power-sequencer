#include "NullInput.hpp"
#include "SignalProvider.hpp"
#include "Signal.hpp"

using namespace std;

// Name returns the instance name
string NullInput::Name(void)
{
	return this->name;
}

NullInput::NullInput(boost::asio::io_context& io, struct ConfigInput* cfg, SignalProvider& prov)
{
	this->out = prov.FindOrAdd(cfg->SignalName);
	this->name = cfg->Name;
}

void NullInput::SetLevel(bool level)
{
	this->out->SetLevel(level);
}

NullInput::~NullInput()
{
}
