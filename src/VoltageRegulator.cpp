
#include "VoltageRegulator.hpp"

#include "SignalProvider.hpp"

#include <boost/filesystem.hpp>

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
        if (!this->alwaysOn)
        {
            this->SetState(this->newLevel ? ENABLED : DISABLED);
        }
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

void VoltageRegulator::ReadStatesSysfs(void)
{
    enum RegulatorStatus status = this->ReadStatus();
    enum RegulatorState state = this->ReadState();

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
    outfile << (state == ENABLED ? "enabled" : "disabled");
    outfile.close();
}

enum RegulatorStatus VoltageRegulator::ReadStatus()
{
    string line;
    ifstream infile(sysfsRoot / path("status"));
    getline(infile, line);
    infile.close();

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
        if (line.compare(lookup[i].str) == 0)
        {
            return lookup[i].status;
        }
    }

    // Invalid state. Error to shut down platform.
    return ERROR;
}

enum RegulatorState VoltageRegulator::ReadState()
{
    string line;
    ifstream infile(sysfsRoot / path("state"));
    getline(infile, line);
    infile.close();

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
        if (line.compare(lookup[i].str) == 0)
        {
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

void VoltageRegulator::Event(inotify::Notification notification)
{
    // FIXME: schedule task on asio instead
    this->ReadStatesSysfs();
}

VoltageRegulator::VoltageRegulator(struct ConfigRegulator* cfg,
                                   SignalProvider& prov, string root) :
    alwaysOn{false},
    active{false}
{
    string consumerRoot;
    this->in = prov.FindOrAdd(cfg->Name + "_On");
    this->in->AddReceiver(this);

    this->enabled = prov.FindOrAdd(cfg->Name + "_Enabled");
    this->fault = prov.FindOrAdd(cfg->Name + "_Fault");
    this->powergood = prov.FindOrAdd(cfg->Name + "_PowerGood");
    this->alwaysOn = cfg->always_on;

    if (this->alwaysOn)
    {
        this->active = true;
        this->newLevel = true;
    }

    if (root == "")
        root = SysFsRootDirByName(cfg->Name);
    if (root == "")
    {
        throw runtime_error("Regulator " + cfg->Name + " not found in sysfs");
    }
    this->sysfsRoot = path(root);
    consumerRoot = SysFsConsumerDir(root);
    if (consumerRoot == "")
    {
        throw runtime_error("reg-userspace-consumer for regulator " +
                            cfg->Name + " not found in sysfs");
    }
    this->sysfsConsumerRoot = path(consumerRoot);

    VoltageRegulator::SetOnInotifyEvent(this);
}

VoltageRegulator::~VoltageRegulator()
{
    boost::lock_guard<boost::mutex> lock(VoltageRegulator::lock);

    // boost::filesystem::path p = this->sysfsRoot / path("state");
    // VoltageRegulator::builder.unwatchFile(p.string());
    // VoltageRegulator::map.erase(p.string());

    // p = this->sysfsRoot / path("status");
    // VoltageRegulator::builder.unwatchFile(p.string());
    // VoltageRegulator::map.erase(p.string());
}
