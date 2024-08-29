#ifndef CS_MATH_H
#define CS_MATH_H

namespace cs_math {
  constexpr int mod(int n, int m) {
    return ((n % m) + m) % m;
  }
} // namespace Math

#endif