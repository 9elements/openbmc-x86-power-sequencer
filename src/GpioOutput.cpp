
#include "GpioOutput.hpp"

using namespace std;

// Name returns the instance name
string GpioOutput::Name(void)
{
	return this->chip.name() + "/" + this->line.name();
}

void GpioOutput::Apply(bool newLevel)
{
	if (newLevel != this->active) {
		 this->active = newLevel;
		 this->line.set_value(newLevel);
	}
}

GpioOutput::GpioOutput(string chipName, string lineName, bool activeLow)
{
	if (chipName == "") {
		for (auto& it: ::gpiod::make_chip_iter()) {
			try {
				this->line = it.find_line(lineName);
				this->chip = it;
				break;
			} catch (const ::std::system_error& exc) {
				continue;
			}
		}
	} else {
		this->chip.open(chipName, gpiod::chip::OPEN_LOOKUP);
		this->line = this->chip.find_line(lineName);
	}

	try {
		if (this->line.name() == "") {
			throw std::runtime_error("GPIO line " + lineName + " not found");
		}
	} catch (std::logic_error& exc) {
		throw std::runtime_error("GPIO line " + lineName + " not found");
	}

	::gpiod::line_request requestOutput = {
		"x86-power-sequencer",
		gpiod::line_request::DIRECTION_OUTPUT,
		activeLow ? gpiod::line_request::FLAG_ACTIVE_LOW : 0

	};
	this->line.request(requestOutput);
}

GpioOutput::GpioOutput(struct ConfigOutput *cfg)
{
	GpioOutput(cfg->GpioChipName, cfg->Name, cfg->ActiveLow);
}

GpioOutput::~GpioOutput()
{
	this->line.release();
}
