
#include <boost/thread/lock_guard.hpp>
#include <boost/filesystem.hpp>
#include <chrono>

#include "SignalProvider.hpp"
#include "IODriver.hpp"

using namespace std;
using namespace std::chrono;

SignalProvider::SignalProvider(Config& cfg) :
	floatingSignals {cfg.FloatingSignals}
{
	for (auto it : cfg.Immutables) {
		Signal *s = this->FindOrAdd(it.SignalName);
		s->SetLevel(it.Level);
	}
}

SignalProvider::~SignalProvider()
{
	for (auto it: this->signals) {
		delete it;
	}
}

// Add a new signal and take ownership of it.
Signal* SignalProvider::Add(string name)
{
	Signal *s;
	s = new Signal(this, name);
	this->signals.push_back(s);
	return s;
}

// Find returns a signal by name. NULL if not found.
Signal* SignalProvider::Find(string name)
{
	for (auto it: this->signals) {
		if (it->SignalName().compare(name) == 0) {
			return it;
		}
	}
	return nullptr;
}

// FindOrAdd returns a signal by name. If not found a new signal is added
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

// ClearDirty removes the dirty bit of all signals and clears the list
void SignalProvider::ClearDirty(void)
{
  boost::lock_guard<boost::mutex> guard(this->lock);
  for (auto it: this->dirty) {
    it->ClearDirty();
  }
  return this->dirty.clear();
}

// SetDirty adds the signal to the dirty listt
void SignalProvider::SetDirty(Signal *sig)
{
  boost::lock_guard<boost::mutex> guard(this->lock);
  // FIXME: use hashmap
  for (auto it: this->dirty) {
	  if (it == sig) {
	    return;
	  }
  }
  this->dirty.push_back(sig);

  // Invoke dirty bit listeners
  this->dirtyBitSignal();
}

// RegisterDirtyBitEvent adds an event handler for dirty bit set events
void SignalProvider::RegisterDirtyBitEvent(std::function<void (void)> const& lamda)
{
	this->dirtyBitSignal.connect(lamda);
}

void SignalProvider::Validate(std::vector<InputDriver *> drvs)
{
	// Check if signal drives something
	for (auto it: this->signals) {
		it->Validate(this->floatingSignals);
	}

	// For each signal try to find a input driver that sources the signal
	for (auto it: this->signals) {
		bool found = false;
		for (auto d : drvs) {
			for (auto s : d->Signals()) {
				if (s == it) {
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
		if (!found)
			throw std::runtime_error("no one drives signal " + it->SignalName());
	}
}

void SignalProvider::DumpSignals(string p)
{
 	boost::filesystem::path root(p);
	static bool once;
	static long long start;
	if (!once) {
		start = duration_cast< microseconds >(high_resolution_clock::now().time_since_epoch()).count();
		once = true;
	}

	for (auto it: this->signals) {
		boost::filesystem::path f(p / boost::filesystem::path(it->SignalName() + ".txt"));
		std::ofstream outfile(f.string(), std::ofstream::out | std::ofstream::app);
		outfile << duration_cast< microseconds >(high_resolution_clock::now().time_since_epoch()).count() - start;
		outfile << " " << (it->GetLevel() ? 1 : 0) << std::endl;

		outfile.close();
	}
}



