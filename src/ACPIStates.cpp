

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
} RequestedStates[5] = {{
                            .l = ACPI_G3,
                            .signal = "ACPI_STATE_REQ_G3",
                            .name = "G3",
                        },
                        {
                            .l = ACPI_S5,
                            .signal = "ACPI_STATE_REQ_S5",
                            .name = "S5",
                        },
                        {
                            .l = ACPI_S4,
                            .signal = "ACPI_STATE_REQ_S4",
                            .name = "S4",
                        },
                        {
                            .l = ACPI_S3,
                            .signal = "ACPI_STATE_REQ_S3",
                            .name = "S3",
                        },
                        {
                            .l = ACPI_S0,
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

    this->Update();
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
    for (auto it : ObservedStates)
    {
        Signal* s = this->sp->Find(it.signal);
        if (!s)
            continue;
        if (s->GetLevel())
        {
            if (it.l == ACPI_S0 && this->requestedState != ACPI_G3)
                this->dbus.SetHostState(dbus::HostState::running);
            else if (it.l == ACPI_S3 && this->requestedState != ACPI_G3)
                this->dbus.SetHostState(dbus::HostState::standby);
            else if (it.l == ACPI_S4 && this->requestedState != ACPI_G3)
                this->dbus.SetHostState(dbus::HostState::standby);
            else if (it.l == ACPI_S5 && this->requestedState != ACPI_G3)
                this->dbus.SetHostState(dbus::HostState::standby);
            else if (it.l == ACPI_G3 && this->requestedState != ACPI_G3)
                this->dbus.SetHostState(dbus::HostState::transitionToRunning);
            else if (it.l == ACPI_G3 && this->requestedState == ACPI_G3)
                this->dbus.SetHostState(dbus::HostState::off);
            else if (this->requestedState == ACPI_G3)
                this->dbus.SetHostState(dbus::HostState::transitionToOff);
        };

        string l = "on";
        if (!s->GetLevel())
            l = "off";
        log_debug("ACPI State " + it.name + " is " + l);
    }
}

bool ACPIStates::RequestedHostTransition(const std::string& requested,
                                         std::string& resp)
{
    if (requested == "xyz.openbmc_project.State.Host.Transition.Off")
    {
        for (auto it : this->inputs)
            it.second->SetLevel(it.first == ACPI_G3);
    }
    else if (requested == "xyz.openbmc_project.State.Host.Transition.On")
    {
        // FIXME: Remove all signals excepti ACPI_G3????
        for (auto it : this->inputs)
            it.second->SetLevel(it.first == ACPI_S5);
    }
    else
    {
        log_err("Unrecognized host state transition request." + requested);
        throw std::invalid_argument("Unrecognized Transition Request");
        return false;
    }
    resp = requested;
    return true;
}

bool ACPIStates::RequestedPowerTransition(const std::string& requested,
                                          std::string& resp)
{
    if (requested == "xyz.openbmc_project.State.Chassis.Transition.Off")
    {
        // ACPI_STATE_REQ_G3 1
        for (auto it : this->inputs)
            it.second->SetLevel(it.first == ACPI_G3);
    }
    else if (requested == "xyz.openbmc_project.State.Chassis.Transition.On")
    {
        // ACPI_STATE_REQ_G3 0
        for (auto it : this->inputs)
            it.second->SetLevel(it.first == ACPI_S5);
    }
    else if (requested ==
             "xyz.openbmc_project.State.Chassis.Transition.PowerCycle")
    {
        // ACPI_STATE_REQ_G3 1
        // wait for ACPI_STATE_IS_G3
        // ACPI_STATE_REQ_G3 0
    }
    else
    {
        log_err("Unrecognized chassis state transition request." + requested);
        throw std::invalid_argument("Unrecognized Transition Request");

        return false;
    }
    resp = requested;
    return true;
}

ACPIStates::ACPIStates(Config& cfg, SignalProvider& sp,
                       boost::asio::io_service& io) :
    sp{&sp},
    dbus{cfg, io}, requestedState(ACPI_G3)
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
    this->dbus.RegisterRequestedHostTransition(
        [this](const std::string& requested, std::string& resp) {
            log_debug("RequestedHostTransition to " + requested);
            return this->RequestedHostTransition(requested, resp);
        });
    this->dbus.RegisterRequestedPowerTransition(
        [this](const std::string& requested, std::string& resp) {
            log_debug("RequestedPowerTransition to " + requested);
            return this->RequestedPowerTransition(requested, resp);
        });
    this->dbus.Initialize();
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
