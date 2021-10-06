
#include "SignalProvider.hpp"

#include "IODriver.hpp"

#include <boost/filesystem.hpp>
#include <boost/thread/lock_guard.hpp>

#include <chrono>
#include <filesystem>

using namespace std;
using namespace std::chrono;
using namespace std::filesystem;

SignalProvider::SignalProvider(Config& cfg, string dumpFolder) :
    floatingSignals{cfg.FloatingSignals}, dumpFolder{dumpFolder}
{
    for (auto it : cfg.Immutables)
    {
        Signal* s = this->FindOrAdd(it.SignalName);
        s->SetLevel(it.Level);
    }
    if (dumpFolder != "")
    {
        path f(dumpFolder / path("signals.txt"));
        this->outfile.open(f.string(), std::ofstream::out | std::ofstream::app);
    }
}

SignalProvider::~SignalProvider()
{

    if (this->outfile.is_open())
    {
        this->outfile.close();

        std::string cmd =
            "gnuplot -e \"unset ytics; set grid x; set xtics rotate 1000; set yrange [-0.5:" +
            std::to_string(this->signals.size() * 2 + 1) +
            "]; set xlabel 'microseconds'; set term svg size 4096 " +
            std::to_string(256 * this->signals.size()) +
            " dynamic ; set output 'output.svg'; set key outside; ";
        cmd += "plot for [col=2:" + std::to_string(this->signals.size() + 1) +
               "] '" + this->dumpFolder +
               "/signals.txt' using 1:col with lines title columnheader;";

        cout << cmd << endl;
        cmd += "\"";
        system(cmd.c_str());
    }
    for (auto it : this->signals)
    {
        delete it.second;
    }

    this->signals.clear();
    this->dirty.clear();
}

// Add a new signal and take ownership of it.
Signal* SignalProvider::Add(string name)
{
    Signal* s;
    s = new Signal(this, name);

    // All signals start dirty. Track them now for first statemachine invokation.
    this->dirty.push_back(s);

    this->signals[name] = s;
    return s;
}

// Find returns a signal by name. NULL if not found.
Signal* SignalProvider::Find(string name)
{
    auto search = signals.find(name);
    if (search == signals.end())
        return nullptr;

    return this->signals[name];
}

// FindOrAdd returns a signal by name. If not found a new signal is added
Signal* SignalProvider::FindOrAdd(string name)
{
    Signal* s;
    s = this->Find(name);
    if (s != nullptr)
        return s;
    return this->Add(name);
}

std::vector<Signal*> SignalProvider::DirtySignals()
{
    boost::lock_guard<boost::mutex> guard(this->lock);
    return this->dirty;
}

// ClearDirty removes the dirty bit of all signals and clears the list
void SignalProvider::ClearDirty(void)
{
    boost::lock_guard<boost::mutex> guard(this->lock);
    for (auto it : this->dirty)
    {
        it->ClearDirty();
    }
    return this->dirty.clear();
}

// SetDirty adds the signal to the dirty listt
void SignalProvider::SetDirty(Signal* sig)
{
    boost::lock_guard<boost::mutex> guard(this->lock);

    // FIXME: Remove once signal dirty state is thread safe
    for (auto it : this->dirty)
    {
        if (it == sig)
        {
            return;
        }
    }
    this->dirty.push_back(sig);

    // Invoke dirty bit listeners
    this->dirtyBitSignal();
}

// RegisterDirtyBitEvent adds an event handler for dirty bit set events
void SignalProvider::RegisterDirtyBitEvent(
    std::function<void(void)> const& lamda)
{
    this->dirtyBitSignal.connect(lamda);
}

void SignalProvider::Validate(std::vector<SignalDriver*> drvs)
{
    // Check if signal drives something
    for (auto it : this->signals)
    {
        it.second->Validate(this->floatingSignals);
    }

    // For each signal try to find a input driver that sources the signal
    for (auto it : this->signals)
    {
        bool found = false;
        for (auto d : drvs)
        {
            for (auto s : d->Signals())
            {
                if (s == it.second)
                {
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        if (!found)
            throw std::runtime_error("no one drives signal " +
                                     it.second->SignalName());
    }
}

void SignalProvider::DumpSignals(void)
{
    static bool once;
    static long long start;
    if (!once)
    {
        start = duration_cast<microseconds>(
                    high_resolution_clock::now().time_since_epoch())
                    .count();
        once = true;

        outfile << "time"
                << " ";
        // Print columns headers
        for (auto it : this->signals)
        {
            outfile << it.first << " ";
        }
        outfile << endl;
    }

    this->outfile << duration_cast<microseconds>(
                         high_resolution_clock::now().time_since_epoch())
                             .count() -
                         start;
    this->outfile << " ";

    int i = 0;
    for (auto it : this->signals)
    {
        outfile << (it.second->GetLevel() ? (i + 1) : i) << " ";
        i += 2;
    }
    this->outfile << std::endl;
}
