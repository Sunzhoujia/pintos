#include <cassert>
#include <stdint.h>
#include "fixed-point.h"

using namespace std;

void test_add() {
  int32_t x, y, n, m, result;
  
  /* test n + m */
  n = 1, m = 2;
  result = add_fp(int_to_fp(n), int_to_fp(m));
  assert(result == ((n + m) * F));
  

  /* test x + n */
  x = 100, n = 1;
  result = add_fp_and_int(x, n);
  assert(result == (x + n * F));
}

void test_sub() {
  int32_t x, y, n, m, result;

  /* test n - m */
  n = 1, m = 2;
  result = sub_fp(int_to_fp(n), int_to_fp(m));
  assert(result == (n - m) * F);
  

  /* test x - n */
  x = 100, n = 1;
  result = sub_fp_and_int(x, n);
  assert(result == (x - n * F));  

}

void test_multiply() {
  int32_t x, y ,n, result;

  /* test multiply x by y */
  x = 16111, y = 16111;
  result = multi_fp(x, y);
  assert(result == 15842);

  /* test multiply x by n */
  x = F, n = 10;
  result = multi_fp_int(x, n);
  assert(result == (n *F));
}


void test_divide() {
  int32_t x, y, n, m, result;

  /* test divide x by y */
  x = 100 * F, y = F;
  result = div_fp(x, y);
  assert(result == (100 * F));

  /* test divide x by n */
  x = 100 * F, n = 1;
  result = div_fp_int(x, n);
  assert(result == (100 * F));

  /* test round up. */
  x = 2, y = 3;
  assert(round_fp_to_int(div_fp(x, y)) == 1);

}

int main() {
  test_add();
  test_sub();
  test_multiply();
  test_divide();
}