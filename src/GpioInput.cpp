
#include "GpioInput.hpp"
#include "SignalProvider.hpp"
#include "Signal.hpp"

using namespace std;

// Name returns the instance name
string GpioInput::Name(void)
{
	return this->chip.name() + "/" + this->line.name();
}

void GpioInput::Poll(const boost::system::error_code& e)
{
	this->out->SetLevel(this->line.get_value() != 0);
}

GpioInput::GpioInput(boost::asio::io_context& io, struct ConfigInput *cfg, SignalProvider& prov) :
	timer(io, boost::asio::chrono::microseconds(100))
{
	if (cfg->GpioChipName == "") {
		for (auto& it: ::gpiod::make_chip_iter()) {

			try {
				this->line = it.find_line(cfg->Name);
				this->chip = it;
				break;
			} catch (const ::std::system_error& exc) {
				continue;
			}
		}
	} else {
		this->chip.open(cfg->GpioChipName, gpiod::chip::OPEN_LOOKUP);
		this->line = this->chip.find_line(cfg->Name);
	}

	try {
		if (this->line.name() == "") {
			throw std::runtime_error("GPIO line " + cfg->Name + " not found");
		}
	} catch (std::logic_error& exc) {
		throw std::runtime_error("GPIO line " + cfg->Name + " not found");
	}

	this->out = prov.FindOrAdd(cfg->SignalName);

	::gpiod::line_request requestOutput = {
		"x86-power-sequencer",
		gpiod::line_request::DIRECTION_INPUT,
		cfg->ActiveLow ? gpiod::line_request::FLAG_ACTIVE_LOW : 0

	};
	this->line.request(requestOutput);
	//FIXME: GPIO event wait support
	this->timer.async_wait([&](const boost::system::error_code& e){
		this->Poll(e);
	});
}

GpioInput::~GpioInput()
{
	this->line.release();
}
