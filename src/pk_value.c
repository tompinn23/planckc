#include "pk_value.h"

#include <math.h>

int pk_value_dbl_to_int(double d, long* i, FloatEqMode mode) { long f = floor(d);
if (d != f) {
    if (mode == F2Ieq)
      return 0;
    else if (mode == F2Iceil)
      f += 1;
  }
*i = f;
 return 1;
}