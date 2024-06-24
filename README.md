# CLI and input file parameter parsing demo

This <s>is</s> will be a proof of concept for a system where model parameters can be
provided in an input file with TOMLish syntax and/or on the command line
and stored in a C++ struct. Parameters can be type- and range-validated
when the data is read as opposed to when it is used.

The demo uses C++23 facilities, but the functionality can be implemented
in C++17 with a bit of fiddling.

Argument handling outside of the parameter set is handled by a header-library
called tinyopt that I've just copied in from [another project](https://github.com/halfflat/tinyopt).

**Big caveat:** Demo actually isn't complete yet, let alone compiles.
May spin this out as a separate header-only library with tests.

## Goals

* Early validation of parameter values.

* Maximum typage.

* Allow a clean split between model description data and execution
  parameters.

## General organisation

* A parameter reader takes a source of input data (e.g. file, command line
  arguments), a parameter set, and a struct by reference to hold the results.
  Failures to parse could be returned in an error value or just generate an
  exception as done in this demo.

* A parameter set is a collection of parameters all of which are parameterized
  over the same results struct.

* A parameter provides a function that takes a string_view representation of
  a parameter value and assigns it to a field of a results stuct or else
  returns an error code.

* Parameters are defined by the field name, the pointer to member pointer,
  an optional parser/validator, and an optional long description.

* Parsers are functionals that map a string_view of the value representation
  to either the corresponding value or an error code.

* Validators are functionals that map a value to either another value
  (actually, ideally the same value to avoid confusion) or an error code.

## Code snippet

Note, this is taken from `demo.cc` and it's just not ready yet.

```
    struct model_parameters {
        std::string name;
        std::optional<int> count;
        double sub_quux = 1.0;
    };

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

    // command line handling
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
    catch // ...
// [...]

```


## Some possible variations

* Arbitrary value handlers could be provided instead of struct member pointers,
  but it might be veering into enough rope territory.

* Formatting of error feedback is currently in the domain of the reader
  functions rather than in the parameter specifications; instead parsers and validators
  could return an error string rather than an error code and the reader functions
  would incorporate that verbatim in their error output.

* Instead of the input file reader putting everything into one struct, using
  one parameter set, it could perhaps be given a table mapping section names
  to section-specific structs/parameter sets.

* Parameter values aren't typed in the INI-style representation — it is the
  responsibility of the specified or default parser in the parameter definition.
  Syntax typing could be enforced and parameter parsers passed instead a
  discriminated union of the representable types.


