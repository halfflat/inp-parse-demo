#pragma once

// Represents all model parameters; subject of parameter i/o.


struct orbital {
    unsigned n; // ≥ 1
    unsigned l;
};

struct orbital_basis {
    std::vector<unsigned> max_pqn; // indexed by l

    unsigned size() const { return max_pqn.size(); }
    unsigned operator[](unsigned l) const { return max_pqn.at(l); }
};

struct model_config {
    std::string id;
    unsigned Z;

    // Basis

    orbital_basis basis_valence_basis;
    orbital_basis basis_frozen_core;
};

