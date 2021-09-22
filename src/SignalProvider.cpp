
#include "SignalProvider.hpp"

using namespace std;

SignalProvider::SignalProvider()
{
}

SignalProvider::~SignalProvider()
{
	for (auto it: this->signals) {
		delete it;
	}
}

Signal* SignalProvider::Add(string name)
{
	Signal *s;
	s = new Signal(name);
	this->signals.push_back(s);
	return s;
}

Signal* SignalProvider::Find(string name)
{
	for (auto it: this->signals) {
		if (it->SignalName().compare(name) == 0) {
			return it;
		}
	}
	return NULL;
}

Signal* SignalProvider::FindOrAdd(string name)
{
	Signal *s;
	s = this->Find(name);
	if (s != NULL)
		return s;
	return this->Add(name);
}

void SignalProvider::Validate()
{
	for (auto it: this->signals) {
		it->Validate();
	}
}

void SignalProvider::DumpSignals(string path)
{
}



