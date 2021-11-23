
#include "GpioOutput.hpp"

#include "Logging.hpp"
#include "SignalProvider.hpp"

using namespace std;

// Name returns the instance name
string GpioOutput::Name(void)
{
    return this->chip.name() + "/" + this->line.name();
}

void GpioOutput::Apply(void)
{
    if (this->newLevel != this->active)
    {
        this->active = this->newLevel;
        LOGDEBUG("output gpio " + this->Name() + " changed to " +
                 to_string(this->active));
        this->line.set_value(this->newLevel);
    }
}

void GpioOutput::Update(void)
{
    this->newLevel = this->in->GetLevel();
}

GpioOutput::GpioOutput(struct ConfigOutput* cfg, SignalProvider& prov)
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
            catch (const ::system_error& exc)
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
            throw runtime_error("GPIO line " + cfg->Name + " not found");
        }
    }
    catch (logic_error& exc)
    {
        throw runtime_error("GPIO line " + cfg->Name + " not found");
    }

    this->in = prov.FindOrAdd(cfg->SignalName);
    this->in->AddReceiver(this);

    ::gpiod::line_request requestOutput = {
        "x86-power-sequencer", gpiod::line_request::DIRECTION_OUTPUT,
        cfg->ActiveLow ? gpiod::line_request::FLAG_ACTIVE_LOW : 0

    };
    this->line.request(requestOutput);

    this->active = this->line.get_value() > 0;

    LOGDEBUG("using gpio " + this->Name() + " as output ");
}

GpioOutput::~GpioOutput()
{
    this->line.release();
}
