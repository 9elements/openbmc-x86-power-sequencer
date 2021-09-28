
#include <boost/filesystem.hpp>

#include "VoltageRegulator.hpp"
#include "SignalProvider.hpp"

using namespace std;

// Name returns the instance name
string VoltageRegulator::Name(void)
{
	return this->name;
}

void VoltageRegulator::Apply(void)
{
	if (this->newLevel != this->active) {
		this->active = this->newLevel;
		if (!this->alwaysOn) {
			// FIXME

		}
	}
}

std::vector<Signal *> VoltageRegulator::Signals(void)
{
	std::vector<Signal *> vec;

	vec.push_back(this->enabled);
	vec.push_back(this->powergood);
	vec.push_back(this->fault);

	return vec;
}

void VoltageRegulator::Update(void)
{
	this->newLevel = this->in->GetLevel();
}

VoltageRegulator::VoltageRegulator(struct ConfigRegulator *cfg, SignalProvider& prov) 
{
	this->in = prov.FindOrAdd(cfg->Name + "_On");
	this->in->AddReceiver(this);

	this->enabled = prov.FindOrAdd(cfg->Name + "_Enabled");
	this->fault = prov.FindOrAdd(cfg->Name + "_Fault");
	this->powergood = prov.FindOrAdd(cfg->Name + "_PowerGood");
	this->alwaysOn = cfg->always_on;

	if (this->alwaysOn) {
		this->active = true;
		this->newLevel = true;
	}
	boost::filesystem::path p("/sys/class/regulator");
	boost::filesystem::directory_iterator it{p};
	while (it != boost::filesystem::directory_iterator{})
		std::cout << *it++ << '\n';

}

VoltageRegulator::~VoltageRegulator()
{
}
