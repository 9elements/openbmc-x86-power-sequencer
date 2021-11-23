
#include "VoltageRegulator.hpp"

#include "Logging.hpp"
#include "SignalProvider.hpp"
#include "SysFsWatcher.hpp"

#include <filesystem>
#include <fstream>

using namespace std;
using namespace std::filesystem;

// Name returns the instance name
string VoltageRegulator::Name(void)
{
    return this->name;
}

void VoltageRegulator::Apply(void)
{
    if (this->newLevel != this->active)
    {
        this->active = this->newLevel;
        this->SetState(this->newLevel ? ENABLED : DISABLED);
    }
}

vector<Signal*> VoltageRegulator::Signals(void)
{
    vector<Signal*> vec;

    vec.push_back(this->enabled);
    vec.push_back(this->powergood);
    vec.push_back(this->fault);

    return vec;
}

void VoltageRegulator::Update(void)
{
    this->newLevel = this->in->GetLevel();
}

void VoltageRegulator::DecodeStatesSysfs(string status_arg, string state_arg,
                                         string events_arg)
{
    enum RegulatorStatus status = this->DecodeStatus(status_arg);
    enum RegulatorState state = this->DecodeState(state_arg);
    unsigned long events = this->DecodeRegulatorEvent(events_arg);

    // First check events. They might be outdated already.
    // By reading events they get cleared and will not be visible on the next
    // call to this function.
    if (events & REGULATOR_EVENT_FAILURE)
    {
        this->powergood->SetLevel(false);
        this->fault->SetLevel(true);
    }
    if (state == DISABLED)
    {
        this->enabled->SetLevel(false);
        this->powergood->SetLevel(false);
    }
    else
    { /* Enabled or unknown */
        if (status == OFF)
        {
            this->enabled->SetLevel(false);
            this->powergood->SetLevel(false);
            this->fault->SetLevel(false);
        }
        else if (status == ERROR)
        {
            // Errors might get cleared by interrupt handlers before we can read
            // them...
            this->enabled->SetLevel(true);
            this->powergood->SetLevel(false);
            this->fault->SetLevel(true);
        }
        else
        {
            this->enabled->SetLevel(true);
            this->powergood->SetLevel(true);
            this->fault->SetLevel(false);
        }
    }
}

void VoltageRegulator::SetState(const enum RegulatorState state)
{
    ofstream outfile(sysfsConsumerRoot / path("state"));
    if (state == ENABLED)
        LOGDEBUG("enabled regulator " + this->name);
    else
        LOGDEBUG("disabled regulator " + this->name);

    outfile << (state == ENABLED ? "enabled" : "disabled");
    outfile.close();
}

string VoltageRegulator::ReadStatus()
{
    string line;
    ifstream infile(sysfsRoot / path("status"));
    getline(infile, line);
    infile.close();

    return line;
}

unsigned long VoltageRegulator::DecodeRegulatorEvent(string state)
{
    return stoul(state);
}

enum RegulatorStatus VoltageRegulator::DecodeStatus(string state)
{
    static const struct
    {
        enum RegulatorStatus status;
        string str;
    } lookup[7] = {
        {OFF, "off"},         {ON, "on"},         {ERROR, "error"},
        {FAST, "fast"},       {NORMAL, "normal"}, {IDLE, "idle"},
        {STANDBY, "standby"},
    };

    for (int i = 0; i < 7; i++)
    {
        if (state == lookup[i].str || state == lookup[i].str + "\n")
        {
            LOGDEBUG("regulator " + this->name + " status is " + lookup[i].str);

            return lookup[i].status;
        }
    }

    // Invalid state. Error to shut down platform.
    return ERROR;
}

string VoltageRegulator::ReadEvents()
{
    string line;
    ifstream infile(sysfsConsumerRoot / path("events"));
    getline(infile, line);
    infile.close();

    return line;
}

string VoltageRegulator::ReadState()
{
    string line;
    ifstream infile(sysfsRoot / path("state"));
    getline(infile, line);
    infile.close();

    return line;
}

enum RegulatorState VoltageRegulator::DecodeState(string state)
{
    static const struct
    {
        enum RegulatorState state;
        string str;
    } lookup[3] = {
        {ENABLED, "enabled"},
        {DISABLED, "disabled"},
        {UNKNOWN, "unknown"},
    };

    for (int i = 0; i < 3; i++)
    {
        if (state == lookup[i].str || state == lookup[i].str + "\n")
        {
            LOGDEBUG("regulator " + this->name + " state is " + lookup[i].str);

            return lookup[i].state;
        }
    }

    // Invalid state.
    return UNKNOWN;
}

static string SysFsRootDirByName(string name)
{
    path root("/sys/class/regulator");
    directory_iterator it{root};
    while (it != directory_iterator{})
    {
        path p = *it / path("name");
        try
        {
            ifstream infile(p);
            if (infile.is_open())
            {
                string line;
                getline(infile, line);
                infile.close();

                if (line.compare(name) == 0)
                    return it->path().string();
            }
        }
        catch (exception e)
        {}
        it++;
    }
    return "";
}

// SysFsConsumerDir returns the sysfs path to the first consumer that is of
// type reg-userspace-consumer
static string SysFsConsumerDir(path root)
{
    directory_iterator it{root};
    while (it != directory_iterator{})
    {
        path p = *it / path("modalias");
        try
        {
            ifstream infile(p);
            if (infile.is_open())
            {
                string line;
                getline(infile, line);
                infile.close();

                if (line.find("reg-userspace-consumer") != string::npos)
                    return it->path().string();
            }
        }
        catch (exception e)
        {}
        it++;
    }
    return "";
}

VoltageRegulator::VoltageRegulator(boost::asio::io_context& io,
                                   struct ConfigRegulator* cfg,
                                   SignalProvider& prov, string root) :
    active{false},
    eventsShadow("0"), name(cfg->Name)
{
    string consumerRoot;
    this->in = prov.FindOrAdd(cfg->Name + "_On");
    this->in->AddReceiver(this);

    this->enabled = prov.FindOrAdd(cfg->Name + "_Enabled");
    this->fault = prov.FindOrAdd(cfg->Name + "_Fault");
    this->powergood = prov.FindOrAdd(cfg->Name + "_PowerGood");

    if (root == "")
        root = SysFsRootDirByName(cfg->Name);
    if (root == "")
    {
        throw runtime_error("Regulator " + cfg->Name + " not found in sysfs");
    }
    LOGDEBUG("Sysfs path of regulator" + cfg->Name + " is " + root);
    this->sysfsRoot = path(root);
    consumerRoot = SysFsConsumerDir(root);
    if (consumerRoot == "")
    {
        throw runtime_error("reg-userspace-consumer for regulator " +
                            cfg->Name + " not found in sysfs");
    }
    LOGDEBUG("Consumer sysfs path of regulator" + cfg->Name + " is " +
             consumerRoot);
    this->sysfsConsumerRoot = path(consumerRoot);

    // Set initial signal levels
    this->statusShadow = this->ReadStatus();
    this->stateShadow = this->ReadState();

    this->DecodeStatesSysfs(this->statusShadow, this->stateShadow,
                            this->eventsShadow);

    // Register sysfs watcher
    SysFsWatcher* sysw = GetSysFsWatcher(io);
    sysw->Register(this->sysfsRoot / path("state"), [&](filesystem::path p,
                                                        const char* data) {
        this->stateShadow = std::string(data);
        LOGDEBUG("sysfsnotify event on path " + p.string() + ", data " +
                 stateShadow);
        this->DecodeStatesSysfs(this->statusShadow, this->stateShadow,
                                this->eventsShadow);
    });
    sysw->Register(this->sysfsRoot / path("status"), [&](filesystem::path p,
                                                         const char* data) {
        this->statusShadow = std::string(data);
        LOGDEBUG("sysfsnotify event on path " + p.string() + ", data " +
                 statusShadow);
        this->DecodeStatesSysfs(this->statusShadow, this->stateShadow,
                                this->eventsShadow);
    });
    if (filesystem::exists(this->sysfsConsumerRoot / path("events")))
    {
        this->eventsShadow = this->ReadEvents();
        sysw->Register(this->sysfsConsumerRoot / path("events"),
                       [&](filesystem::path p, const char* data) {
                           this->eventsShadow = std::string(data);
                           LOGDEBUG("sysfsnotify event on path " + p.string() +
                                    ", data " + eventsShadow);
                           this->DecodeStatesSysfs(this->statusShadow,
                                                   this->stateShadow,
                                                   this->eventsShadow);
                       });
    }
}

VoltageRegulator::~VoltageRegulator()
{}
