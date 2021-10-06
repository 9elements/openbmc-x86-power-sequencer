
#include "Config.hpp"
#include "IODriver.hpp"
#include "Signal.hpp"

#include <inotify-cpp/NotifierBuilder.h>

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

class VoltageRegulator : SignalReceiver, public OutputDriver, public SignalDriver
{
  public:
    // Name returns the instance name
    string Name(void);

    // Apply sets the new output state
    void Apply(void);

    // Update fetches the current state from connected signals
    void Update(void);

    // Signals returns the list of signals that are feed with data
    std::vector<Signal*> Signals(void);

    VoltageRegulator(struct ConfigRegulator* cfg, SignalProvider& prov,
                     string root = "");
    ~VoltageRegulator();

  private:
    // SysFsRootDirByName returns "/sys/class/regulator/..." path of matching
    // regulator
    std::string SysFsRootDirByName(std::string name);

    // ReadStatus parses /sys/class/regulator/.../status
    enum RegulatorStatus ReadStatus(void);

    // ReadState parses /sys/class/regulator/.../state
    enum RegulatorState ReadState(void);

    // ReadStatesSysfs updates the signals from sysfs attributes
    void ReadStatesSysfs(void);

    // SetState writes to /sys/class/regulator/.../state
    void SetState(const enum RegulatorState state);

    static void SetOnInotifyEvent(VoltageRegulator* reg)
    {
        boost::lock_guard<boost::mutex> lock(VoltageRegulator::lock);

        path p = reg->sysfsRoot / path("state");
        VoltageRegulator::builder.watchFile(p.string());
        VoltageRegulator::map[p.string()] = reg;

        p = reg->sysfsRoot / path("status");
        VoltageRegulator::builder.watchFile(p.string());
        VoltageRegulator::map[p.string()] = reg;

        static bool once;
        if (!once)
        {
            once = true;
            VoltageRegulator::builder.onEvent(
                inotify::Event::modify, [&](inotify::Notification n) {
                    if (n.event == inotify::Event::modify)
                        VoltageRegulator::InotifyEvent(n);
                });
            VoltageRegulator::thread =
                std::thread([&]() { VoltageRegulator::builder.run(); });
            VoltageRegulator::thread.detach();
        }
    }

    void Event(inotify::Notification notification);

    static void InotifyEvent(inotify::Notification n)
    {
        boost::lock_guard<boost::mutex> lock(VoltageRegulator::lock);
        VoltageRegulator::map[n.path]->Event(n);
    }

    string name;
    path sysfsRoot;

    bool active;
    bool newLevel;
    bool hasfault;
    bool alwaysOn;

    Signal* in;
    Signal* enabled;
    Signal* fault;
    Signal* powergood;

    inline static inotify::NotifierBuilder builder;
    inline static std::unordered_map<std::string, VoltageRegulator*> map;
    inline static boost::mutex lock;
    inline static std::thread thread;
};
