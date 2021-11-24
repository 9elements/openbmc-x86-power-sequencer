
#include "Dbus.hpp"

static std::string hostDbusName = "xyz.openbmc_project.State.Host";
static std::string chassisDbusName = "xyz.openbmc_project.State.Chassis";

using namespace dbus;

static constexpr std::string_view getHostState(const PowerState state)
{
    switch (state)
    {
        case PowerState::on:
        case PowerState::gracefulTransitionToOff:
        case PowerState::gracefulTransitionToCycleOff:
            return "xyz.openbmc_project.State.Host.HostState.Running";
            break;
        case PowerState::waitForPSPowerOK:
        case PowerState::waitForSIOPowerGood:
        case PowerState::off:
        case PowerState::transitionToOff:
        case PowerState::transitionToCycleOff:
        case PowerState::cycleOff:
        case PowerState::checkForWarmReset:
            return "xyz.openbmc_project.State.Host.HostState.Off";
            break;
        default:
            return "";
            break;
    }
};

static constexpr std::string_view getChassisState(const PowerState state)
{
    switch (state)
    {
        case PowerState::on:
        case PowerState::transitionToOff:
        case PowerState::gracefulTransitionToOff:
        case PowerState::transitionToCycleOff:
        case PowerState::gracefulTransitionToCycleOff:
        case PowerState::checkForWarmReset:
            return "xyz.openbmc_project.State.Chassis.PowerState.On";
            break;
        case PowerState::waitForPSPowerOK:
        case PowerState::waitForSIOPowerGood:
        case PowerState::off:
        case PowerState::cycleOff:
            return "xyz.openbmc_project.State.Chassis.PowerState.Off";
            break;
        default:
            return "";
            break;
    }
};

void Dbus::SetPowerState(const dbus::PowerState state)
{
#ifdef WITH_SDBUSPLUSPLUS
    this->hostIface->set_property("CurrentHostState",
                                  std::string(getHostState(state)));

    this->chassisIface->set_property("CurrentPowerState",
                                     std::string(getChassisState(state)));
    // this->chassisIface->set_property("LastStateChangeTime",
    // getCurrentTimeMs());
#endif
}

void Dbus::RegisterRequestedHostTransition(
    const std::function<bool(const std::string& requested, std::string& resp)>&
        handler)
{
#ifdef WITH_SDBUSPLUSPLUS

    this->hostIface->register_property(
        "RequestedHostTransition",
        std::string("xyz.openbmc_project.State.Host.Transition.Off"), handler);
#endif
}

void Dbus::RegisterRequestedPowerTransition(
    const std::function<bool(const std::string& requested, std::string& resp)>&
        handler)
{
#ifdef WITH_SDBUSPLUSPLUS
    this->chassisIface->register_property(
        "RequestedPowerTransition",
        std::string("xyz.openbmc_project.State.Chassis.Transition.Off"),
        handler);

#endif
}

void Dbus::Initialize(void)
{
#ifdef WITH_SDBUSPLUSPLUS
    this->hostIface->initialize();
    this->chassisIface->initialize();
#endif
}

Dbus::Dbus(Config& cfg, boost::asio::io_service& io)
#ifdef WITH_SDBUSPLUSPLUS
    :
    conn{std::make_shared<sdbusplus::asio::connection>(io)},
    hostServer{conn}, chassisServer
{
    conn
}

#endif
{
    string node = "0";
#ifdef WITH_SDBUSPLUSPLUS

    if (node == "0")
    {
        // Request all the dbus names
        conn->request_name(hostDbusName.c_str());
        conn->request_name(chassisDbusName.c_str());
    }

    hostDbusName += node;
    chassisDbusName += node;

    // Request all the dbus names
    conn->request_name(hostDbusName.c_str());
    conn->request_name(chassisDbusName.c_str());

    // Power Control Interface
    this->hostIface =
        hostServer.add_interface("/xyz/openbmc_project/state/host" + node,
                                 "xyz.openbmc_project.State.Host");

    this->hostIface->register_property(
        "CurrentHostState", std::string(getHostState(dbus::PowerState::off)));

    // Chassis Control Interface
    this->chassisIface =
        chassisServer.add_interface("/xyz/openbmc_project/state/chassis" + node,
                                    "xyz.openbmc_project.State.Chassis");

    this->chassisIface->register_property(
        "CurrentPowerState",
        std::string(getChassisState(dbus::PowerState::off)));
    // this->chassisIface->register_property("LastStateChangeTime",
    //                                     getCurrentTimeMs());
#endif
}

Dbus::~Dbus()
{}