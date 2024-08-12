#pragma once

#include "model_config.h"

// io helpers

struct spectroscopic_notation {
    static constexpr char names[]="spdfghiklmnoqrtuv";

    static constexpr char from_l(unsigned l) {
        return l<sizeof(names)? names[l]: '?';
    }

    static constexpr int to_l(char s) {
        s|=32;
        for (unsigned i = 0; i<sizeof(names); ++i) if (s==names[i]) return i;
        return -1;
    }

    static constexpr unsigned max_l = sizeof(names)-1;
};

// interface wrapping parapara etc.


// Throw this with a helpful? error message on parse error.

struct model_config_io_error: std::runtime_error {
    model_config_io_error(const std::string& message): std::runtime_error(message) {}
};

// Update model_config from key=value string; throw on error, but return false on
// unrecognized key.

bool model_config_parse_kv(model_config& M, const std::string& kv);

// Update model_config from data in ini-style file; throw on error.

void model_config_read_ini(model_config& M, const std::string& path);

// Export model_config in ini-style format.

std::ostream& export_model_config(std::ostream& out, model_config& M);

// List parameter keys with descriptions.

std::ostream& list_model_config_parameters(std::ostream& out);


