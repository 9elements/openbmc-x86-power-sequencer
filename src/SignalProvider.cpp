
#include "SignalProvider.hpp"

#include <boost/thread/lock_guard.hpp>

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
	s = new Signal(this, name);
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

std::vector<Signal *> SignalProvider::DirtySignals()
{
  boost::lock_guard<boost::mutex> guard(this->lock);
  return this->dirty;
}

void SignalProvider::ClearDirty(void)
{
  boost::lock_guard<boost::mutex> guard(this->lock);
  for (auto it: this->dirty) {
    it->ClearDirty();
  }
  return this->dirty.clear();
}

void SignalProvider::SetDirty(Signal *sig)
{
  boost::lock_guard<boost::mutex> guard(this->lock);
  for (auto it: this->dirty) {
	  if (it->SignalName() == sig->SignalName())
	    return;
  }
  this->dirty.push_back(sig);
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



