
#include "NullOutput.hpp"
#include "SignalProvider.hpp"

using namespace std;

// Name returns the instance name
string NullOutput::Name(void)
{
	return this->name;
}

void NullOutput::Apply(void)
{
	if (this->newLevel != this->active) {
		this->active = this->newLevel;
	}
}

void NullOutput::Update(void)
{
	this->newLevel = this->in->GetLevel();
}

bool NullOutput::GetLevel(void)
{
	return this->active;
}

NullOutput::NullOutput(SignalProvider& prov, string name, string signalName)
{
	this->in = prov.FindOrAdd(signalName);
	this->in->AddReceiver(this);
	this->newLevel = false;
	this->active = false;
	this->name = name;
}

NullOutput::~NullOutput()
{
}
