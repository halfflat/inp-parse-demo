#include <tinyopt/tinyopt.h>

#include <para/parameter.h>
#include <para/reader.h>

const char* usage_str =
    "[OPTION | PARAM=VALUE]...\n"
    "\n"
    "  -h, --help         Display usage information and exit\n"
    "  -i, --input=FILE   Read parameter values from FILE\n"
    "  --list-parameters  Print a list of valid parameters to stdout and exit\n"
    "\n"
    "Collects a set of parameter values from input files and/or the command\n"
    "line and prints the collated values to stdout.\n";

struct model_parameters {
    std::string name;
    std::optional<int> count;
    double sub_quux = 1.0;
};

int main() {
    model_parameters V;

    para::specification param_specs[] = {
        { "name",
           &model_parameters::name,
          "Name of simulation configuration"
        },
        { "count",
          &model_parameters::count,
          // todo: pretty syntax for composing validators
          [](int x) { return
            para::assert([](int n) { return n>0; })(x).and_then(
            para::assert([](int n) { return n%2!=0; }));
          }
          "Number of repetitions; even, positive integer"
        },
        { "sub/quux",
          &model_parameters::sub_quux,
          "Quuxificatory coefficient"
        }
    };

    para::parameter_set<model_parameters> pset(param_specs, param::keys_lc_nows);

    // Command line argument parsing

    try {
        bool list_parameters = false;

        auto help = [argv0 = argv[0]] { to::usage(argv0, usage_str); };

        auto set_from_kv = [&](std::string kv) { return para::parse_kv_assignment(V, pset, kv); };
        auto read_param_file = [&](std::string file) { return para::parse_ini_file(V, file); }; // unimplemented

        to::option opts[] = {
            { to::action(read_param_file), "-i", "--input" },
            { to::action(help), "-h", "--help", to::exit, to::flag },
            { to::set(list_params), "--list-parameters", to::flag },
            { to::action(parse_param_kv), to::lax }
        };

        if (!to::run(opts, argc, argv+1)) return 0;
        if (argv[1]) throw to::option_error("unrecognized option", argv[1]);

        if (list_params) {
            for (auto& spec: param_specs) {
                std::cout << spec.key << ":\n" << spec.long_desc << "\n\n";
            }
            return 0;
        }
    }
    catch (to::option_error& e) {
        to::usage_error(argv[0], "Use option '--help' for more information", e.what());
        return 1;
    }
    catch (para::value_parse_error& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    catch (para::value_range_error& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // Parameters are all set but we'll just boringly print them out.

    std::cout << "name: " << V.name << '\n';
    if (V.count) std::cout << "count: " << V.count << '\n';
    else std::cout << "count: unset\n";
    std::cout << "sub/quux: " << V.sub_quux << '\n';

    return 0;
}
