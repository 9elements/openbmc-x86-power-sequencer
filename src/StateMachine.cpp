
#include "StateMachine.hpp"

#include <boost/thread/lock_guard.hpp>

#include <functional>
using namespace std;
using namespace placeholders;

// Create statemachine from config
StateMachine::StateMachine(Config& cfg, SignalProvider& prov,
                           boost::asio::io_service& io) :
    io{&io},
    work_guard{io.get_executor()}
{

    prov.SetDirtyBitEvent([&](void) { this->OnDirtySet(); });

    for (int i = 0; i < cfg.Logic.size(); i++)
    {
        Logic* l = new Logic(io, prov, &cfg.Logic[i]);
        this->logic.push_back(l);
        this->signalDrivers.push_back(l);
        std::cout << "pushing logic " << cfg.Logic[i].Name << " to list "
                  << std::endl;
    }
    for (int i = 0; i < cfg.Inputs.size(); i++)
    {
        if (cfg.Inputs[i].InputType == INPUT_TYPE_GPIO)
        {
            GpioInput* g = new GpioInput(io, &cfg.Inputs[i], prov);
            this->gpioInputs.push_back(g);
            std::cout << "pushing gpio input " << cfg.Inputs[i].SignalName
                      << " to list " << std::endl;
            this->signalDrivers.push_back(g);
        }
        else if (cfg.Inputs[i].InputType == INPUT_TYPE_NULL)
        {
            NullInput* n = new NullInput(io, &cfg.Inputs[i], prov);
            this->nullInputs.push_back(n);
            std::cout << "pushing null input " << cfg.Inputs[i].SignalName
                      << " to list " << std::endl;
            this->signalDrivers.push_back(n);
        }
    }

    for (int i = 0; i < cfg.Outputs.size(); i++)
    {
        if (cfg.Outputs[i].OutputType == OUTPUT_TYPE_GPIO)
        {
            GpioOutput* g = new GpioOutput(&cfg.Outputs[i], prov);
            this->gpioOutputs.push_back(g);
            this->outputDrivers.push_back(g);
            std::cout << "pushing gpio output " << cfg.Outputs[i].SignalName
                      << " to list " << std::endl;
        }
        else if (cfg.Outputs[i].OutputType == OUTPUT_TYPE_NULL)
        {
            NullOutput* n = new NullOutput(&cfg.Outputs[i], prov);
            this->nullOutputs.push_back(n);
            std::cout << "pushing null output " << cfg.Outputs[i].SignalName
                      << " to list " << std::endl;
            this->outputDrivers.push_back(n);
        }
    }

    for (int i = 0; i < cfg.Regulators.size(); i++)
    {
        VoltageRegulator* v = new VoltageRegulator(&cfg.Regulators[i], prov);
        this->voltageRegulators.push_back(v);
        this->outputDrivers.push_back(v);
        this->signalDrivers.push_back(v);
    }

    this->sp = &prov;
    this->running = false;
}

StateMachine::~StateMachine(void)
{
    this->work_guard.reset();
}

// Validates checks if the current config is sane
void StateMachine::Validate(void)
{
    this->sp->Validate(this->signalDrivers);
}

// ApplyOutputSignalLevel writes signal state to outputs
void StateMachine::ApplyOutputSignalLevel(void)
{
    for (auto it : this->outputDrivers)
    {
        it->Apply();
    }
}

// OnDirtySet is invoked when a signal dirty bit is set
void StateMachine::OnDirtySet(void)
{
    boost::lock_guard<boost::mutex> lock(this->scheduledLock);
    if (!this->running)
    {
        this->io->post([&]() { this->EvaluateState(); });
    }
}

// EvaluateState runs until no more signals change
// FIXME: An input GPIO might set the dirty bit while this code
// runs. Add an timer for fixed runtime? block inputs while this runs?
void StateMachine::EvaluateState(void)
{
    {
        boost::lock_guard<boost::mutex> lock(this->scheduledLock);
        this->running = true;
    }

    std::vector<Signal*>* signals = this->sp->GetDirtySignalsAndClearList();

    while (signals->size() > 0)
    {
        /* Invoke Update() method of logic units */
        for (auto sig : *signals)
        {
            sig->UpdateReceivers();
        }
        // The Update call might have added new dirty signals.
        // FIXME: Add timeout and loop detection.
        signals = this->sp->GetDirtySignalsAndClearList();
    }

    // State is stable
    // TODO: Dump state
    this->ApplyOutputSignalLevel();
    {
        boost::lock_guard<boost::mutex> lock(this->scheduledLock);
        this->running = false;
    }
}

// Run does work on the io_queue.
void StateMachine::Run(void)
{
    this->io->post([&]() { this->EvaluateState(); });

    this->io->run();
}

void StateMachine::Poll(void)
{
    this->io->poll();
}

std::vector<NullOutput*> StateMachine::GetNullOutputs(void)
{
    return this->nullOutputs;
}

std::vector<NullInput*> StateMachine::GetNullInputs(void)
{
    return this->nullInputs;
}