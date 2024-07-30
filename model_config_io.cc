#include <expected>
#include <iostream>
#include <cctype>
#include <fstream>
#include <string>
#include <string_view>

#include <parapara/parapara.h>
#include "model_config.h"
#include "model_config_io.h"

namespace P = parapara;
using P::hopefully;

P::specification<model_config> specs[] = {
    {"Z", &model_config::Z,
        P::minimum(1u),
        "Atomic number of target system"},
    {"ID", &model_config::id,
        P::require([](const auto& x) { return !x.empty(); }, "value is non-empty"),
        "String identifying the calculation"},
    {"Basis/ValenceBasis", &model_config::basis_valence_basis,
        "Maximum principal quantum number and orbital angular momentum of the orbitals included in CI calculations"},
    {"Basis/FrozenCore", &model_config::basis_frozen_core,
        "Forbid hole excitations in given shells upto given principal quantum number"}
};

P::specification_set<model_config> canonical_specs(specs, P::keys_lc_nows);

hopefully<orbital_basis> read_orbital_basis(std::string_view v) {
    std::vector<unsigned> max_pqn;

    unsigned max_n = 0;
    while (!v.empty()) {
        unsigned p = 0;
        while (std::isdigit(v[p]) && p<v.size()) ++p;

        if (p>0) {
            auto hi = P::read_cc<unsigned>(v.substr(0, p));
            if (!hi) return std::unexpected(hi.error()); // error parsing max_n value

            max_n = hi.value();
            v.remove_prefix(p);
        }

        if (v.empty()) return std::unexpected(P::invalid_value("missing orbital"));

        int l = spectroscopic_notation::to_l(v[0]);
        if (l<0) return std::unexpected(P::invalid_value("unrecognized orbital"));

        if ((unsigned)l >= max_pqn.size()) max_pqn.resize(l+1);
        max_pqn[l] = max_n;

        v.remove_prefix(1);
    }

    return orbital_basis{std::move(max_pqn)};
}

std::string write_orbital_basis(const orbital_basis& B) {
    std::string repn;

    unsigned n = 0;
    for (unsigned l = 0; l<B.size(); ++l) {
        if (B[l]==0) continue;
        if (B[l]!=n) {
            n = B[l];
            repn += P::write_cc<unsigned>(n);
        }
        repn += spectroscopic_notation::from_l(l);
    }

    return repn;
}


P::reader model_readers(P::default_reader(),
                        read_orbital_basis);

P::writer model_writers(P::default_writer(),
                        write_orbital_basis);

// Update model_config from key=value string; throw on error from parsing
// value, but just return false if we don't recognize key.

bool model_config_parse_kv(model_config& M, const std::string& kv) {
    auto h = import_k_eq_v(M, canonical_specs, model_readers, kv);
    if (!h) {
        const P::failure& fail = h.error();
        if (fail.error==P::failure::unrecognized_key) return false;
        else throw model_config_io_error(P::explain(fail));
    }
    return true;
}

// Update model_config from data in ini-style file; throw on error.

void model_config_read_ini(model_config& M, const std::string& path) {
    std::ifstream file(path);
    if (!file) throw model_config_io_error("unable to read file '"+path+"'");

    auto h = P::import_ini(M, canonical_specs, model_readers, file, "/");
    if (!h) {
        P::failure fail = h.error();
        fail.ctx.source = path;
        throw model_config_io_error(P::explain(fail, true));
    }
}

// Export model_config in ini-style format.

std::ostream& export_model_config(std::ostream& out, model_config& M) {
    auto h = P::export_ini(M, specs, model_writers, out, "/");
    if (!h) throw model_config_io_error(P::explain(h.error()));
    return out;
}

// List parameter keys with descriptions.

std::ostream& list_model_config_parameters(std::ostream& out) {
    for (const auto& s: specs) {
        // pinched this from the parapara ini exporter; maybe this should be
        // provided as a handy utility
        std::string_view desc(s.description);
        while (!desc.empty()) {
            auto h = desc.find('\n');
            out << "# " << desc.substr(0, h) << '\n';
            desc.remove_prefix(h==std::string_view::npos? desc.size(): h+1);
        }

        out << s.key << "\n\n";
    }
    return out;
}
