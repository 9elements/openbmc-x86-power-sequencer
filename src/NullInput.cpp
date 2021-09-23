#include "NullInput.hpp"
#include "SignalProvider.hpp"
#include "Signal.hpp"

using namespace std;

// Name returns the instance name
string NullInput::Name(void)
{
	return this->name;
}

NullInput::NullInput(boost::asio::io_context& io, SignalProvider& prov, string name, string signalName)
{
	this->out = prov.FindOrAdd(signalName);
	this->name = name;
}

void NullInput::SetLevel(bool level)
{
	this->out->SetLevel(level);
}

NullInput::~NullInput()
{
}
