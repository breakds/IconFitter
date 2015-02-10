#ifndef _ICON_FITTER_ALGEBRA_
#define _ICON_FITTER_ALGEBRA_

#include <cmath>
#include <iostream>

#define _USE_MATH_DEFINES

namespace icon_fitter {
  namespace algebra {

    constexpr double epsilon = 1e-4;

    // ---------- Geenralized Vector Operations ----------
    // Suitable for generalized vectors (object with operator[]
    // defined).
    
    template <typename VectorType>
    inline void PrintVector(const VectorType &input, int dim) {
      std::cout << "vector: ";
      if (dim > 0) {
        std::cout << '[' << input[0];
        for (int i = 1; i < dim; ++i) {
          std::cout << ", " << input[i];
        }
        std::cout << ']';
      }
      std::cout << '\n';
    }

    
    // ---------- Array-based Vector Operations ----------
    
    template <typename FloatType>
    inline void PlusInPlace(FloatType *original, 
                            const FloatType *addon, 
                            int dim) {
      FloatType *original_p = original;
      const FloatType *addon_p = addon;
      for (int i = 0; i < dim; ++i) {
        *(original_p++) += *(addon_p++);
      }
    }

    template <typename FloatType>
    inline void CopyVector(const FloatType *source, 
                           FloatType *destination, 
                           int dim) {
      memcpy(destination, source, dim * sizeof(FloatType));
    }

    template <typename FloatType>
    inline void MinusInPlace(FloatType *original,
                             const FloatType *minused,
                             int dim) {
      FloatType *original_p = original;
      const FloatType *minused_p = minused;
      for (int i = 0; i < dim; ++i) {
        *(original_p++) -= *(minused_p++);
      }
    }

    template <typename FloatType>
    inline void Normalize(FloatType *input, int dim) {
      FloatType sum = epsilon;
      for (int i = 0; i < dim; ++i) {
        sum += input[i] * input[i];
      }
      sum = 1.0 / sqrt(sum);
      for (int i = 0; i < dim; ++i) {
        input[i] *= sum;
      }
    }
  }  // namespace algebra
}  // icon_fitter

#endif  // _ICON_FITTER_ALGEBRA_
