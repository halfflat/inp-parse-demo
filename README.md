# CLI and input file parameter parsing demo

This is a demo of the use of an [in-development library](https://github.com/halfflat/parapara)
for the parsing and writing of model parameters or other configuration data backed by the fields
of a C++ struct. As this also constitutes a proposal for use within [AMBiT](https://github.com/drjuls/AMBiT),
model parameter examples have been based on some of those used in AMBiT configuration.

The demo highlights the use of both an INI-style text format and command-line arguments
as sources of configuration data and the use of custom readers/writers for data types.
The framework for command-line parsing is [tinyopt](https://github.com/halfflat/tinyopt).



