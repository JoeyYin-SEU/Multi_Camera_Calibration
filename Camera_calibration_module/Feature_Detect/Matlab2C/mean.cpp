//
// Academic License - for use in teaching, academic research, and meeting
// course requirements at degree granting institutions only.  Not for
// government, commercial, or other organizational use.
// File: mean.cpp
//
// MATLAB Coder version            : 5.5
// C/C++ source code generated on  : 19-Dec-2023 13:39:53
//

// Include Files
#include "mean.h"
#include "rt_nonfinite.h"
#include "coder_array.h"

// Function Definitions
//
// Arguments    : const ::coder::array<float, 1U> &x
// Return Type  : float
//
namespace coder {
float mean(const ::coder::array<float, 1U> &x)
{
  float b_x;
  if (x.size(0) == 0) {
    b_x = 0.0F;
  } else {
    int firstBlockLength;
    int lastBlockLength;
    int nblocks;
    if (x.size(0) <= 1024) {
      firstBlockLength = x.size(0);
      lastBlockLength = 0;
      nblocks = 1;
    } else {
      firstBlockLength = 1024;
      nblocks = static_cast<int>(static_cast<unsigned int>(x.size(0)) >> 10);
      lastBlockLength = x.size(0) - (nblocks << 10);
      if (lastBlockLength > 0) {
        nblocks++;
      } else {
        lastBlockLength = 1024;
      }
    }
    b_x = x[0];
    for (int k{2}; k <= firstBlockLength; k++) {
      b_x += x[k - 1];
    }
    for (int ib{2}; ib <= nblocks; ib++) {
      float bsum;
      int hi;
      firstBlockLength = (ib - 1) << 10;
      bsum = x[firstBlockLength];
      if (ib == nblocks) {
        hi = lastBlockLength;
      } else {
        hi = 1024;
      }
      for (int k{2}; k <= hi; k++) {
        bsum += x[(firstBlockLength + k) - 1];
      }
      b_x += bsum;
    }
  }
  return b_x / static_cast<float>(x.size(0));
}

} // namespace coder

//
// File trailer for mean.cpp
//
// [EOF]
//
