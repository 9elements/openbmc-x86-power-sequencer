
#include "ACPIStates.hpp"
#include "Config.hpp"
#include "SignalProvider.hpp"
#include "StateMachine.hpp"

#include <popl.hpp>

#include <iostream>

using namespace std;
using namespace popl;

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

    try
    {
        op.parse(argc, argv);

        // print auto-generated help message
        if (help_option->count() == 1)
            cout << op << "\n";
        else if (help_option->count() == 2)
            cout << op.help(Attribute::advanced) << "\n";
        else if (help_option->count() > 2)
            cout << op.help(Attribute::expert) << "\n";

        if (!config_option->is_set() || config_option->value() == "")
        {
            std::cout << op << '\n';
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
            std::cerr << "Failed to load config:" << std::endl
                      << ex.what() << std::endl;
            return 1;
        }
    }
    catch (const popl::invalid_option& e)
    {
        cerr << "Invalid Option Exception: " << e.what() << "\n";
        cerr << "error:  ";
        if (e.error() == invalid_option::Error::missing_argument)
            cerr << "missing_argument\n";
        else if (e.error() == invalid_option::Error::invalid_argument)
            cerr << "invalid_argument\n";
        else if (e.error() == invalid_option::Error::too_many_arguments)
            cerr << "too_many_arguments\n";
        else if (e.error() == invalid_option::Error::missing_option)
            cerr << "missing_option\n";

        if (e.error() == invalid_option::Error::missing_option)
        {
            string option_name(e.option()->name(OptionName::short_name, true));
            if (option_name.empty())
                option_name = e.option()->name(OptionName::long_name, true);
            cerr << "option: " << option_name << "\n";
        }
        else
        {
            cerr << "option: " << e.option()->name(e.what_name()) << "\n";
            cerr << "value:  " << e.value() << "\n";
        }
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    try
    {
        SignalProvider signalprovider(cfg, dumpSignalsFolder);
        ACPIStates states(cfg, signalprovider);

        StateMachine sm(cfg, signalprovider, io);

        sm.Validate();
        sm.Run();
    }
    catch (const exception& ex)
    {
        std::cerr << "Failed to use provided configuration:" << std::endl
                  << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
