
#include "ACPIStates.hpp"
#include "Config.hpp"
#include "Logging.hpp"
#include "SignalProvider.hpp"
#include "StateMachine.hpp"

#include <popl.hpp>

#include <iostream>

using namespace std;
using namespace popl;

int _loglevel;

int main(int argc, const char* argv[])
{
    Config cfg;
    string dumpSignalsFolder;
    boost::asio::io_service io;
    OptionParser op("Allowed options");
    auto help_option = op.add<Switch>("h", "help", "produce help message");
    auto config_option = op.add<Value<string>>(
        "c", "config", "Path to configuration file/folder.");
    auto dump_signals_option = op.add<Value<string>>(
        "d", "dump_signals_folder", "Path to dump signal.txt [DEBUGGING ONLY]");
    auto verbose = op.add<Value<string>>(
        "v", "verbose", "Enable verbose logging [DEBUGGING ONLY]");
    auto quiet = op.add<Value<string>>("q", "quiet",
                                       "Be quiet and don't log any errors");

    LOGINFO("Starting " + string(argv[0]) + " ....");

    try
    {
        op.parse(argc, argv);

        // print auto-generated help message
        if (help_option->count() == 1)
        {
            LOGINFO(op.help() + "\n");
            return 0;
        }
        else if (help_option->count() == 2)
        {
            LOGINFO(op.help(Attribute::advanced) + "\n");
            return 0;
        }
        else if (help_option->count() > 2)
        {
            LOGINFO(op.help(Attribute::expert) + "\n");
            return 0;
        }

        // Set loglevel
        if (quiet)
            _loglevel = 0;
        else if (verbose)
            _loglevel = 2;
        else
            _loglevel = 1;

        if (!config_option->is_set() || config_option->value() == "")
        {
            LOGINFO(op.help() + "\n");
            return 1;
        }
        if (dump_signals_option->is_set())
            dumpSignalsFolder = dump_signals_option->value();

        try
        {
            cfg = LoadConfig(config_option->value());
        }
        catch (const exception& ex)
        {
            LOGERR("Failed to load config:\n");
            LOGERR(string(ex.what()) + "\n");
            return 1;
        }
    }
    catch (const popl::invalid_option& e)
    {
        LOGERR("Invalid Option Exception: " + string(e.what()) + "\n");
        LOGERR("error:  ");
        if (e.error() == invalid_option::Error::missing_argument)
            LOGERR("missing_argument\n");
        else if (e.error() == invalid_option::Error::invalid_argument)
            LOGERR("invalid_argument\n");
        else if (e.error() == invalid_option::Error::too_many_arguments)
            LOGERR("too_many_arguments\n");
        else if (e.error() == invalid_option::Error::missing_option)
            LOGERR("missing_option\n");

        if (e.error() == invalid_option::Error::missing_option)
        {
            string option_name(e.option()->name(OptionName::short_name, true));
            if (option_name.empty())
                option_name = e.option()->name(OptionName::long_name, true);
            LOGERR("option: " + option_name + "\n");
        }
        else
        {
            LOGERR("option: " + e.option()->name(e.what_name()) + "\n");
            LOGERR("value:  " + e.value() + "\n");
        }
        return EXIT_FAILURE;
    }
    catch (const exception& e)
    {
        LOGERR("Exception: " + string(e.what()) + "\n");
        return EXIT_FAILURE;
    }
    LOGINFO("Loaded config files.");

    try
    {
        SignalProvider signalprovider(cfg, dumpSignalsFolder);
        ACPIStates states(cfg, signalprovider);

        StateMachine sm(cfg, signalprovider, io);

        LOGINFO("Validating config ...");

        sm.Validate();

        LOGINFO("Starting main loop.");

        sm.Run();
    }
    catch (const exception& ex)
    {
        LOGERR("Failed to use provided configuration:\n");
        LOGERR(string(ex.what()) + "\n");
        return 1;
    }

    return 0;
}
