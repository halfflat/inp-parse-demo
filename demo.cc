#include <expected>
#include <iostream>

#include <parapara/parapara.h>
#include <tinyopt/tinyopt.h>

#include "model_config.h"
#include "model_config_io.h"

const char* usage_str =
    "[OPTION | PARAM=VALUE]...\n"
    "\n"
    "  -h, --help         Display usage information and exit\n"
    "  -i, --input=FILE   Read parameter values from FILE\n"
    "  --list-parameters  Print a list of valid parameters to stdout and exit\n"
    "\n"
    "Collects a set of parameter values from input files and/or the command\n"
    "line and prints the collated values to stdout.\n";


int main(int argc, char** argv) {
    model_config M;
    bool list_parameters = false;

    try {
        auto help = [argv0 = argv[0]] { to::usage(argv0, usage_str); };
        auto set_from_kv = [&](std::string kv) { return model_config_parse_kv(M, kv); };
        auto read_param_file = [&](std::string file) { return model_config_read_ini(M, file); };

        to::option opts[] = {
            { to::action(read_param_file), "-i", "--input" },
            { to::action(help), "-h", "--help", to::exit, to::flag },
            { to::set(list_parameters), "--list-parameters", to::flag },
            { to::action(set_from_kv), to::lax }
        };

        if (!to::run(opts, argc, argv+1)) return 0;
        if (argv[1]) throw to::option_error("unrecognized option", argv[1]);
    }
    catch (to::option_error& e) {
        to::usage_error(argv[0], "Use option '--help' for more information", e.what());
        return 1;
    }
    catch (model_config_io_error& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    if (list_parameters) {
        list_model_config_parameters(std::cout);
        return 0;
    }

    export_model_config(std::cout, M);
    return 0;
}
