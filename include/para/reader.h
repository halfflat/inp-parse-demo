#pragma once

#include <cctype>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <para/paraexcept.h>
#include <para/parameter.h>

namespace para {

// Some canonicalizers to try out, none of which even attempt
// to do the right thing by unicode or any other encoding really.

std::string keys_lc(std::string_view v) {
    using std::views::transform;

    std::string out;
    std::ranges::copy(
        v |
        transform([](unsigned char c) { return std::tolower(c); }),
        std::back_inserter(out));
    return out;
}

std::string keys_lc_nows(std::string_view v) {
    using std::views::transform;
    using std::views::filter;

    std::string out;
    std::ranges::copy(v |
        filter([](unsigned char c) { return !std::isspace(c); }) |
        transform([](unsigned char c) { return std::tolower(c); }),
        std::back_inserter(out));
    return out;
}

// A parameter set is a key-indexed collection of specifications where the keys are
// optionally canonicalized by a supplied map std::string -> std::string.
// Creating a parameter_set with duplicate keys after canonicalization will
// throw a bad_parameter_set exception.

template <typename R>
struct parameter_set {
    parameter_set() = default;

    template <typename Collection> // imagine I've put a requires clause on this
    explicit parameter_set(const Collection& c, std::function<std::string (std::string_view)> cify = {}):
        canonicalize_(cify)
    {
        for (const specification<R>& spec: c) {
            std::string k = canonicalize(c.key);
            auto [_, inserted] = set_.emplace(k, spec);

            if (!inserterd) throw bad_parameter_set("degenerate canonicalized key");
        }
    }

    std::string canonicalize(std::string_view v) const { return canonicalize_? canonicalize_(v): std::string(v); }

    bool contains(std::string_view key) const { return set_.contains(canonicalize(key)); }
    const specification<R>& at(std::string_view key) const { return set_.at(canonicalize(key)); }
    const specification<R>* get_if(std::string_view key) const {
        auto i = set_.find(canonicalize(key));
        if (i==set_.end()) return nullptr;
        else return &(*i);
    }

private:
    std::map<std::string, specification<R>> set_;
    std::function<std::string (std::string_view) canonicalize_;
};


// These just throw on parse/validation errors but could instead do something with e.g. std::expected.
// Otherwise: return true if the pset contains the key (and the assignment works), else false.

// Argument string should strictly be in the format key=value and whitespace is not explicitly stripped.
// If there is no '=' in the argument, the key is regarded as representing a boolean parameter and
// the value string 'true' is used.

template <typename R>
bool parse_kv_assignment(R& record, const parameter_set<V>& pset, std::string_view kv) {
    std::string_view k(kv), v(kv);

    auto eq = kv.find('=');
    if (eq == kv.npos) {
        v = std::string_view("true");
    }
    else {
        k.remove_suffix(k.size()-eq);
        v.remove_prefix(eq+1);
    }

    const specification<R>* sp = pset.get_if(k);
    if (!sp) return false;

    if (auto r = sp->assign(record, v)) return true;

    switch (r.error) {
    case bad_value::parse_error:
        throw value_parse_error("failed to parse value", k, v);
    case bad_value::range_error:
        throw value_range_error("value not in range", k, v);
    default:
        throw value_error("unexpected error assigning value");
    }
}

} // namespace param
