#include <stdexcept>
#include <string>

namespace para {

// Base exception class; other errors derive from this.

struct para_error: public std::runtime_error {
    parameter_error(const std::string& message): std::runtime_error(message) {}
};

// Thrown when creating parameter sets, e.g. because of a degenerate key set,

struct bad_parameter_set: para_error {
    bad_parameter_set(const std::string& message): para_error(message) {}
};

// Thrown by a parameter reader when encountering misformed input.

struct syntax_error: para_error {
   syntax_error(const std::string& message): para_error(message) {}
};

// Readers, if not collating and returning value erros to the caller, can
// throw value_parse_error if the corresponding parameter fails to parse
// the value string, or value_range_error if the parsed value fails
// a constraint imposed by the parameter.
//
// Could add more info to these, e.g. input data context.

struct value_error: para_error {
   value_error(const std::string& message):
        para_error(message) {}

   value_error(const std::string& message, std::string key, std::string value):
        para_error(message), key(std::move(key)), value(std::move(value)) {}

    std::string key, value;
};

struct value_parse_error: value_error {
   value_parse_error(const std::string& message):
        value_error(message) {}

   value_parse_error(const std::string& message, std::string key, std::string value):
        value_error(message, std::move(key), std::move(value)) {}
};

struct value_range_error: value_error {
   value_range_error(const std::string& message):
        value_error(message) {}

   value_range_error(const std::string& message, std::string key, std::string value):
        value_error(message, std::move(key), std::move(value)) {}
};

} // namespace para
