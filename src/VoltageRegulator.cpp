
#include "VoltageRegulator.hpp"

#include "Logging.hpp"
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
    if (state == ENABLED)
        LOGDEBUG("enabled regulator " + this->name);
    else
        LOGDEBUG("disabled regulator " + this->name);

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
            LOGDEBUG("regulator " + this->name + " status is " + lookup[i].str);

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

void VoltageRegulator::Event(inotify::Notification notification)
{
    // FIXME: schedule task on asio instead
    LOGDEBUG("Inotify event for " + notification.path.string());
    this->ReadStatesSysfs();
}

VoltageRegulator::VoltageRegulator(boost::asio::io_context& io,
                                   struct ConfigRegulator* cfg,
                                   SignalProvider& prov, string root) :
    active{false},
    descState(io), descStatus(io)
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

    VoltageRegulator::SetOnInotifyEvent(this);

    path p = this->sysfsRoot / path("state");
    this->fState = fopen(p.string().c_str(), "r");
    if (!this->fState)
    {
        std::cout << "failed to open path - " << p.string() << std::endl;
    }
    else
    {
        char buf[32];

        this->descState.assign(fileno(this->fState));
        LOGDEBUG("Read fd is " + to_string(fileno(this->fState)));

        this->descState.read_some(boost::asio::buffer(buf));
        LOGDEBUG("Read some of " + p.string());
    }

    p = this->sysfsRoot / path("status");
    this->fStatus = fopen(p.string().c_str(), "r");
    if (!this->fStatus)
    {
        std::cout << "failed to open path - " << p.string() << std::endl;
    }
    else
    {
        char buf[32];

        this->descStatus.assign(fileno(this->fStatus));
        LOGDEBUG("Read fd is " + to_string(fileno(this->fStatus)));
        this->descStatus.read_some(boost::asio::buffer(buf));
        LOGDEBUG("Read some of " + p.string());
    }
    std::function<void(error_code)> status_loop = [&](error_code ec) {
        if (!ec)
        {
            cout << "status_loop " << endl;
            fseek(this->fStatus, 0, SEEK_SET);
            char recv_str[32] = {};
            try
            {
                this->descStatus.read_some(boost::asio::buffer(recv_str));
                cout << "read " << recv_str << endl;
            }
            catch (const exception& ex)
            {
                LOGERR("read_some throwed exception: " + string(ex.what()));
            }

            cout << "boost asio event " << p.string() << endl;
            this->descStatus.async_wait(
                boost::asio::posix::descriptor::wait_type::wait_read,
                status_loop);
        }
    };
    this->descStatus.async_wait(
        boost::asio::posix::descriptor::wait_type::wait_read, status_loop);

    if (0)
    {
        SetAsyncWaitEvent(
            this->sysfsRoot / path("state"), this->descState,
            [&](boost::asio::posix::stream_descriptor& event, path p) {
                char recv_str[1024] = {};
                event.read_some(boost::asio::buffer(recv_str));
                cout << "read " << recv_str << endl;
                cout << "boost asio event " << p.string() << endl;
            });

        SetAsyncWaitEvent(
            this->sysfsRoot / path("status"), this->descStatus,
            [&](boost::asio::posix::stream_descriptor& event, path p) {
                char recv_str[1024] = {};
                event.read_some(boost::asio::buffer(recv_str));
                cout << "read " << recv_str << endl;
                cout << "boost asio event " << p.string() << endl;
            });
    }
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
