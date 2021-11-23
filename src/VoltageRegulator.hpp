
#include "Config.hpp"
#include "IODriver.hpp"
#include "Logging.hpp"
#include "Signal.hpp"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/lock_guard.hpp>

#include <filesystem>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace std::filesystem;

class SignalProvider;
class Signal;

enum RegulatorState
{
    ENABLED = 0,
    DISABLED = 1,
    UNKNOWN = 2,
};

enum RegulatorStatus
{
    OFF = 0,
    ON = 1,
    ERROR = 2,
    FAST = 3,
    NORMAL = 4,
    IDLE = 5,
    STANDBY = 6,
};

class VoltageRegulator :
    SignalReceiver,
    public OutputDriver,
    public SignalDriver
{
  public:
    // Name returns the instance name
    string Name(void);

    // Apply sets the new output state
    void Apply(void);

    // SignalReceiver's Update method for signal changes
    void Update(void);

    // Signals returns the list of signals that are feed with data
    vector<Signal*> Signals(void);

    VoltageRegulator(boost::asio::io_context& io, struct ConfigRegulator* cfg,
                     SignalProvider& prov, string root = "");
    ~VoltageRegulator();

  private:
    // ReadStatus parses /sys/class/regulator/.../status
    enum RegulatorStatus ReadStatus(void);

    // ReadState parses /sys/class/regulator/.../state
    enum RegulatorState ReadState(void);

    // ReadStatesSysfs updates the signals from sysfs attributes
    void ReadStatesSysfs(void);

    // SetState writes to /sys/class/regulator/.../state
    void SetState(const enum RegulatorState state);

    string name;
    path sysfsRoot;
    path sysfsConsumerRoot;

    bool active;
    bool newLevel;
    bool hasfault;

    Signal* in;
    Signal* enabled;
    Signal* fault;
    Signal* powergood;
};
