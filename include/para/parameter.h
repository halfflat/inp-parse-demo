#pragma once

#include <charconv>
#include <expected>
#include <string>
#include <stringstream>
#include <string_view>

namespace para {

// Maybe bad_value should be a bit richer so that it can carry meaningful
// error reports from parsers/validators.

enum struct bad_value: int {
    parse_error = 1,
    range_error = 2
};

// Nothing fancy here yet: strip leading and trailing whitespace and then use
// from_chars if we can to get a number, or else istream::operator>> to read.

template <typename U>
struct default_parser {
    static std::expected<U, bad_value> parse(std::string_view) { return std::unexpected(bad_value::parse_error); }
};

inline std::string_view trim_ws(std::string_view sv) {
    auto b = sv.find_first_not_of(" \t");
    auto e = sv.find_last_not_of(" \t");
    if (b==sv.npos || e==sv.npos) return {};

    sv.remove_prefix(b);
    sv.remove_suffix(sv.size()+1-e);
    return sv;
}

template <typename U>
requires (requires (U x, char *p) { std::from_chars(p, p, x); })
struct default_parser {
    static std::expected<U, bad_value> parse(std::string_view sv) {
        U u;
        sv = trim_ws(sv);
        auto [p, err] = std::from_chars(&sv.front(), &sv.front()+sv.size(), u);

        if (err==std::errc::result_out_of_range) return std::unexpected(bad_value::range_error);
        if (err!=std::errc() || p!=&sv.front()+sv.size()) return std::unexpected(bad_value::parse_error);
        return u;
    }
};

template <typename U>
requires (requires (U x, std::istream &in) { in >> x; })
struct default_parser {
    static std::expected<U, bad_value> parse(std::string_view sv) {
        U u;
        std::istringstream is(std::string(trim_ws(sv)));
        is.setf(std::ios_base::boolalpha);

        is >> u;
        if (!is || (is.peek(), !is.eof())) return std::unexpected(bad_value::parse_error);
        return u;
    }
};

template <typename U>
std::expected<U, bad_value> default_parser(std::string_view sv) {
    return default_parser_impl<U>::parse(sv);
}

template <typename P, typename U>
concept ParameterParserFor = requires (P p, U u) {
    {p(std::declval<std::string_view>())} -> std::convertible_to<std::expected<U, bad_value>>;
}

template <typename V, typename U>
concept ParameterValidatorFor = requires (V v, U u) {
    {v(std::declval<U>())} -> std::convertible_to<std::expected<U, bad_value>>;
}

template <typename T>
struct specification {
    std::string key;
    std::string description;

    template <typename U, typename P>
    requires ParameterParserFor<P, U>
    specification(std::string key, U T::* field, P parser, std::string description = ""):
        key(std::move(key)),
        assign_(
            [field = field, parser = std::move(parser)] (T& record, std::string_view sv) {
                if (auto r = parser(sv)) {
                    record.*field = std::move(r);
                    return std::expected<void, bad_value>();
                }
                else return std::unexpected(r.error());
            }),
        description(description)
    {}

    template <typename U, typename V>
    requires ParameterValidatorFor<V, U>
    specification(std::string key, U T::* field, V vtor, std::string description = ""):
        param_spec(
            std::move(key),
            field,
            [v = std::move(vtor)](std::string_view sv) { return default_parser<U>(sv).and_then(v); },
            description)
    {}

    template <typename U>
    specification(std::string key, U T::* field, std::string description = ""):
        param_spec(std::move(key), field, default_parser<U>, description)
    {}

    std::expected<void, bad_value> assign(T& record, std::string_view sv) { return assign_.(record, sv); }

private:
    std::function<std::expected<void, bad_value> (T&, std::string_view)> assign_;
};



} // namespace para
