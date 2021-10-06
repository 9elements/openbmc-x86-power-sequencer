
#include "GpioInput.hpp"

#include "Signal.hpp"
#include "SignalProvider.hpp"

using namespace std;

// Name returns the instance name
string GpioInput::Name(void)
{
    return this->chip.name() + "/" + this->line.name();
}

// Poll needs to be called in case the async GPIO events aren't supported
void GpioInput::Poll(const boost::system::error_code& e)
{
    this->out->SetLevel(this->line.get_value() != 0);
}

// OnEvent is called by the async handler whenever an GPIO event has occured
void GpioInput::OnEvent(gpiod::line_event line_event)
{
    this->out->SetLevel(line_event.event_type ==
                        gpiod::line_event::RISING_EDGE);
}

// WaitForGPIOEvent async waits for an event on the open gpiod fd
void GpioInput::WaitForGPIOEvent(void)
{
    this->streamDesc.async_wait(
        boost::asio::posix::stream_descriptor::wait_read,
        [&](const boost::system::error_code ec) {
            if (ec)
            {
                std::string errMsg =
                    this->Name() + " fd handler error: " + ec.message();
                cout << errMsg << endl;
                // TODO: throw here to force power-control to restart?
                return;
            }
            gpiod::line_event line_event = this->line.event_read();
            this->OnEvent(line_event);
            this->WaitForGPIOEvent();
        });
}

GpioInput::GpioInput(boost::asio::io_context& io, struct ConfigInput* cfg,
                     SignalProvider& prov) :
    streamDesc(io)
{
    if (cfg->GpioChipName == "")
    {
        for (auto& it : ::gpiod::make_chip_iter())
        {

            try
            {
                this->line = it.find_line(cfg->Name);
                this->chip = it;
                break;
            }
            catch (const ::std::system_error& exc)
            {
                continue;
            }
        }
    }
    else
    {
        this->chip.open(cfg->GpioChipName, gpiod::chip::OPEN_LOOKUP);
        this->line = this->chip.find_line(cfg->Name);
    }

    try
    {
        if (this->line.name() == "")
        {
            throw std::runtime_error("GPIO line " + cfg->Name + " not found");
        }
    }
    catch (std::logic_error& exc)
    {
        throw std::runtime_error("GPIO line " + cfg->Name + " not found");
    }

    this->out = prov.FindOrAdd(cfg->SignalName);

    ::gpiod::line_request requestOutput = {
        "x86-power-sequencer", gpiod::line_request::DIRECTION_INPUT,
        cfg->ActiveLow ? gpiod::line_request::FLAG_ACTIVE_LOW : 0

    };
    this->line.request(requestOutput);
    int gpioLineFd = this->line.event_get_fd();
    if (gpioLineFd < 0)
    {
        std::string errMsg = "Failed to get fd for gpio line " + cfg->Name;
        throw std::runtime_error(errMsg);
    }

    this->streamDesc.assign(gpioLineFd);

    this->WaitForGPIOEvent();

    // Read initial level once ready
    io.post([&] { this->out->SetLevel(this->line.get_value() != 0); });
}

std::vector<Signal*> GpioInput::Signals(void)
{
    std::vector<Signal*> vec;
    vec.push_back(this->out);
    return vec;
}

GpioInput::~GpioInput()
{
    this->line.release();
}
