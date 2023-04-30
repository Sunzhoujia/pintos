#ifndef FIXED_POINT_H
#define FIXED_POINT_H


#include <stdint.h>


#define F (1 << 14)


/** Convert n to fixed point. */
static inline int32_t int_to_fp(int32_t n) {
  return n * F;
}

/** Convert x to integer, round toward zero. */
static inline int32_t fp_to_int(int32_t x) {
  return x / F;
}

/** Convert x to integer, round toward nearest. */
static inline int32_t round_fp_to_int(int32_t x) {
  if (x >= 0) {
    return (x + F / 2) / F;
  } else {
    return (x - F  / 2) / F;
  }
}

/** Add x and y. */
static inline int32_t add_fp(int32_t x, int32_t y) {
  return x + y;
}

/** Sub y from x. */
static inline int32_t sub_fp(int32_t x, int32_t y) {
  return x - y;
}

/** Add x and n. */
static inline int32_t add_fp_and_int(int32_t x, int32_t n) {
  return x + n * F;
}

/** Sub n from x. */
static inline int32_t sub_fp_and_int(int32_t x, int32_t n) {
  return x - n * F;
}

/** Multiply x by y. */
static inline int32_t multi_fp(int32_t x, int32_t y) {
  return ((int64_t)x * y) / F;
}

/** Multiply x by n. */
static inline int32_t multi_fp_int(int32_t x, int32_t n) {
  return x * n;
}

/** Divide x by y. */
static inline int32_t div_fp(int32_t x, int32_t y) {
  return ((int64_t)x) * F / y;
}

/** Divide x by n. */
static inline int32_t div_fp_int(int32_t x, int32_t n) {
  return x / n;
}

#endif