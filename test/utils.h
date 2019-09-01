#pragma once

#include <catch2/catch.hpp>

#include <sstream>

template<typename T>
class equals : public Catch::MatcherBase<T> {
    T value_;

public:
    explicit equals(T value) : value_(std::move(value)) {}

    virtual bool match(const T& to_match) const override {
        return value_ == to_match;
    }

    virtual std::string describe() const override {
        std::ostringstream os;
        os << "equals \"" << value_ << '"';
        return os.str();
    }
};

template <typename T>
inline auto Equals(T value) {
    return equals<T>(std::move(value));
}

inline auto Equals(const char* str) {
    return equals<std::string_view>(str);
}

namespace std {

template<typename Ch, typename T1, typename T2>
basic_ostream<Ch, char_traits<Ch>>&
operator<<(basic_ostream<Ch, char_traits<Ch>>& os, const std::pair<T1, T2>& p) {
    return os << '{' << get<0>(p) << ';' << get<1>(p) << '}';
}

}
