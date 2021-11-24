#pragma once

#include "Config.hpp"

#include <boost/asio.hpp>

#ifdef WITH_SDBUSPLUSPLUS
#include <sdbusplus/asio/object_server.hpp>
#endif

using namespace std;

namespace dbus
{

enum class PowerState
{
    on,
    waitForPSPowerOK,
    waitForSIOPowerGood,
    off,
    transitionToOff,
    gracefulTransitionToOff,
    cycleOff,
    transitionToCycleOff,
    gracefulTransitionToCycleOff,
    checkForWarmReset,
};

static std::string getPowerStateName(PowerState state)
{
    switch (state)
    {
        case PowerState::on:
            return "On";
            break;
        case PowerState::waitForPSPowerOK:
            return "Wait for Power Supply Power OK";
            break;
        case PowerState::waitForSIOPowerGood:
            return "Wait for SIO Power Good";
            break;
        case PowerState::off:
            return "Off";
            break;
        case PowerState::transitionToOff:
            return "Transition to Off";
            break;
        case PowerState::gracefulTransitionToOff:
            return "Graceful Transition to Off";
            break;
        case PowerState::cycleOff:
            return "Power Cycle Off";
            break;
        case PowerState::transitionToCycleOff:
            return "Transition to Power Cycle Off";
            break;
        case PowerState::gracefulTransitionToCycleOff:
            return "Graceful Transition to Power Cycle Off";
            break;
        case PowerState::checkForWarmReset:
            return "Check for Warm Reset";
            break;
        default:
            return "unknown state: " + std::to_string(static_cast<int>(state));
            break;
    }
}
} // namespace dbus

// The Dbus class handles the DBUS interface
class Dbus
{
  public:
    Dbus(Config& cfg, boost::asio::io_service& io);
    ~Dbus();
    void SetPowerState(const dbus::PowerState);
    void RegisterRequestedHostTransition(
        const std::function<bool(const std::string& requested,
                                 std::string& resp)>& handler);

    void RegisterRequestedPowerTransition(
        const std::function<bool(const std::string& requested,
                                 std::string& resp)>& handler);
    void Initialize(void);

  private:
#ifdef WITH_SDBUSPLUSPLUS
    std::shared_ptr<sdbusplus::asio::connection> conn;

    sdbusplus::asio::object_server hostServer;
    sdbusplus::asio::object_server chassisServer;

    std::shared_ptr<sdbusplus::asio::dbus_interface> hostIface;
    std::shared_ptr<sdbusplus::asio::dbus_interface> chassisIface;
#endif
};
