

#include "ACPIStates.hpp"

#include "Config.hpp"
#include "Logging.hpp"
#include "Signal.hpp"

using namespace std;

static const struct
{
    enum ACPILevel l;
    string signal;
    string name;
} RequestedStates[5] = {
    {
        .l = ACPI_G3,
        .signal = "ACPI_STATE_REQ_G3",
        .name = "G3",
    },
    {
        .l = ACPI_S5,
        .signal = "ACPI_STATE_REQ_S5",
        .name = "S5",
    },
    {.l = ACPI_S4, .signal = "ACPI_STATE_REQ_S4", .name = "S4"},
    {
        .l = ACPI_S3,
        .signal = "ACPI_STATE_REQ_S3",
        .name = "S3",
    },
    {
        .l = ACPI_S5,
        .signal = "ACPI_STATE_REQ_S0",
        .name = "S0",
    }};

static const struct
{
    enum ACPILevel l;
    string signal;
    string name;
} ObservedStates[5] = {{
                           .l = ACPI_G3,
                           .signal = "ACPI_STATE_IS_G3",
                           .name = "G3",
                       },
                       {
                           .l = ACPI_S5,
                           .signal = "ACPI_STATE_IS_S5",
                           .name = "S5",
                       },
                       {
                           .l = ACPI_S4,
                           .signal = "ACPI_STATE_IS_S4",
                           .name = "S4",
                       },
                       {
                           .l = ACPI_S3,
                           .signal = "ACPI_STATE_IS_S3",
                           .name = "S3",
                       },
                       {
                           .l = ACPI_S0,
                           .signal = "ACPI_STATE_IS_S0",
                           .name = "S0",
                       }};

// Request tells the logic which on is the desired power state
// The power logic and platform might take a while to change the
// power state, or even fail.
void ACPIStates::Request(enum ACPILevel l)
{
    this->requestedState = l;

    for (auto it : this->inputs)
    {
        it.second->SetLevel(it.first == l);
    }
}

// GetRequested returns the requested ACPI state.
enum ACPILevel ACPIStates::GetRequested(void)
{
    return this->requestedState;
}

// GetCurrent returns the current ACPI state.
enum ACPILevel ACPIStates::GetCurrent(void)
{
    int levels = 0;
    for (auto it : this->outputs)
    {
        if (it.second->GetLevel())
            levels++;
    }
    if (levels > 1)
        return ACPI_INVALID;

    for (auto it : this->outputs)
    {
        if (it.second->GetLevel())
            return it.first;
    }

    return ACPI_INVALID;
}

void ACPIStates::Update(void)
{
    // FIXME: Notify dbus
    for (auto it : ObservedStates)
    {
        Signal* s = this->sp->Find(it.signal);
        if (!s)
            continue;
        string l = "on";
        if (!s->GetLevel())
            l = "off";
        LOGDEBUG("ACPI State " + it.name + " is " + l);
    }
}

ACPIStates::ACPIStates(Config& cfg, SignalProvider& sp) : sp{&sp}
{
    for (auto c : cfg.ACPIStates)
    {
        for (auto it : RequestedStates)
        {
            if (c.Name == it.name)
            {
                Signal* s = this->sp->FindOrAdd(it.signal);
                this->inputs[it.l] = s;

                s->SetLevel(c.Initial);
                break;
            }
        }
        for (auto it : ObservedStates)
        {
            if (c.Name == it.name)
            {
                Signal* s = this->sp->FindOrAdd(it.signal);
                this->outputs[it.l] = s;
                s->AddReceiver(this);
            }
        }
    }
}

ACPIStates::~ACPIStates()
{
    for (auto it : RequestedStates)
    {
        this->inputs.erase(it.l);
    }
    for (auto it : ObservedStates)
    {
        this->outputs.erase(it.l);
    }
}

vector<Signal*> ACPIStates::Signals()
{
    vector<Signal*> vec;

    for (auto it : this->inputs)
    {
        vec.push_back(it.second);
    }

    return vec;
}
