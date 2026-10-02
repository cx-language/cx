#pragma once

#include <cstdint>

namespace cx {

// Safety checks the backend emits, individually disableable with attributes
// (`@noOverflowCheck`, `@noBoundsCheck`, `@noNullCheck`, or `@unchecked` for
// all of them) on functions, statements, and expressions.
enum class DisabledChecks : uint8_t {
    None = 0,
    Overflow = 1,
    Bounds = 2,
    Null = 4,
};

constexpr DisabledChecks operator|(DisabledChecks left, DisabledChecks right) {
    return DisabledChecks(uint8_t(left) | uint8_t(right));
}

constexpr DisabledChecks& operator|=(DisabledChecks& left, DisabledChecks right) {
    left = left | right;
    return left;
}

constexpr bool disablesCheck(DisabledChecks mask, DisabledChecks check) {
    return (uint8_t(mask) & uint8_t(check)) != 0;
}

constexpr DisabledChecks allDisabledChecks = DisabledChecks::Overflow | DisabledChecks::Bounds | DisabledChecks::Null;

} // namespace cx
