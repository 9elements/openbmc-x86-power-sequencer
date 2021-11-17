
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
    auto verbose = op.add<Switch>("v", "verbose",
                                  "Enable verbose logging [DEBUGGING ONLY]");
    auto quiet =
        op.add<Switch>("q", "quiet", "Be quiet and don't log any errors");

    _loglevel = 1;

    LOGINFO("Starting " + string(argv[0]) + " ....");

    try
    {
        op.parse(argc, argv);

        // print auto-generated help message
        if (help_option->count() == 1)
        {
            cout << op.help() << endl;
            return 0;
        }
        else if (help_option->count() == 2)
        {
            cout << op.help(Attribute::advanced) << endl;
            return 0;
        }
        else if (help_option->count() > 2)
        {
            cout << op.help(Attribute::expert) << endl;
            return 0;
        }

        // Set loglevel
        if (quiet->is_set())
            _loglevel = 0;
        else if (verbose->is_set())
            _loglevel = 2;

        if (!config_option->is_set() || config_option->value() == "")
        {
            LOGERR("Didn't specify a valid config file");
            LOGERR(op.help());
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
            LOGERR("Failed to load config: " + string(ex.what()));
            return 1;
        }
    }
    catch (const popl::invalid_option& e)
    {
        LOGERR("Invalid Option Exception: " + string(e.what()));
        LOGERR("error:  ");
        if (e.error() == invalid_option::Error::missing_argument)
            LOGERR("missing_argument");
        else if (e.error() == invalid_option::Error::invalid_argument)
            LOGERR("invalid_argument");
        else if (e.error() == invalid_option::Error::too_many_arguments)
            LOGERR("too_many_arguments");
        else if (e.error() == invalid_option::Error::missing_option)
            LOGERR("missing_option");

        if (e.error() == invalid_option::Error::missing_option)
        {
            string option_name(e.option()->name(OptionName::short_name, true));
            if (option_name.empty())
                option_name = e.option()->name(OptionName::long_name, true);
            LOGERR("option: " + option_name);
        }
        else
        {
            LOGERR("option: " + e.option()->name(e.what_name()));
            LOGERR("value:  " + e.value());
        }
        return EXIT_FAILURE;
    }
    catch (const exception& e)
    {
        LOGERR("Exception: " + string(e.what()));
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
        LOGERR("Failed to use provided configuration: " + string(ex.what()));
        return 1;
    }

    return 0;
}
