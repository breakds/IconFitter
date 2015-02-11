#ifndef _ICON_FITTER_PATCHMATCH_
#define _ICON_FITTER_PATCHMATCH_

#include <random>
#include <chrono>
#include <tuple>

#include "algebra.h"

namespace icon_fitter {
  
  struct Transform {
    int y = 0;
    int x = 0;
  };
  
  template <typename DataType>
  struct DataMatrix {
    int height;
    int width;

    DataMatrix(int height_, int width_) 
      : height(height_), width(width_) {
      matrix_.resize(height * width);
    }

    DataMatrix(DataMatrix<DataType> &&other) = default;
    DataMatrix<DataType> &operator=(DataMatrix<DataType> &&other) = default;

    DataType &operator()(int i, int j) {
      return matrix_[i * width + j];
    }

    inline const std::vector<DataType> &data() {
      return matrix_;
    }
    
  private:
    std::vector<DataType> matrix_;
  };

  typedef DataMatrix<Transform> TransformMap;

  struct PatchMatchOptions {
    int initial_candidates = 5;
    int iterations = 10;

    // The shrinking rate of random search radius.
    double decay_rate = 0.5;

    // If the update rate is below this threshold, terminate the
    // algroithm.
    double termination_update_rate = 0.05;
  };

  namespace {
    struct PatchMatchControl {
      int y_begin;
      int y_delta;
      int y_end;
      int y_max;
      int x_begin;
      int x_delta;
      int x_end;
      int x_max;
      
      PatchMatchControl(int height, int width)
        : y_max(height - 1), x_max(width - 1) {
        std::tie(y_begin, y_delta, y_end) = std::make_tuple(0, 1, y_max + 1);
        std::tie(x_begin, x_delta, x_end) = std::make_tuple(0, 1, x_max + 1);
      }

      void Switch() {
        std::tie(y_begin, y_delta, y_end) = 
          std::make_tuple(y_max - y_begin, -y_delta, y_max - y_end);
        std::tie(x_begin, x_delta, x_end) = 
          std::make_tuple(x_max - x_begin, -x_delta, x_max - x_end);
      }
    };

    struct BoundaryChecker {
      int height;
      int width;
      
      inline bool operator()(int y, int x) {
        return 0 <= y && y < height && 0 <= x && x < width;
      }
    };
  }  // namespace

  template <typename DataType, typename Distance = algebra::L2>
  TransformMap PatchMatch(BlockFeatureImage<DataType> &source, 
                          BlockFeatureImage<DataType> &target,
                          PatchMatchOptions options) {

    if (source.dimension != target.dimension) {
      printf("[ERROR] dimension mismatch between source and target.");
      exit(-1);
    }
    // Initialize Random Generator
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    
    TransformMap result(target.height, target.width);
    DataMatrix<double> score_map(target.height, target.width);
    
    // Initialization 
    std::uniform_int_distribution<int> y_random(0, source.height - 1);
    std::uniform_int_distribution<int> x_random(0, source.width - 1);
    auto y_dice = std::bind(y_random, generator);
    auto x_dice = std::bind(x_random, generator);
    for (int i = 0; i < target.height; ++i) {
      for (int j = 0; j < target.width; ++j) {
        double &score = score_map(i, j);
        Transform &transform = result(i, j);
        for (int k = 0; k < options.initial_candidates; ++k) {
          int y = y_dice();
          int x = x_dice();
          double current_score = Distance::Compute(target.GetPatch(i, j),
                                                   source.GetPatch(y, x));
          if (0 == k || current_score < score) {
            score = current_score;
            transform.y = y - i;
            transform.x = x - j;
          }
        }
      }
    }


    // Iteration Preparation
    PatchMatchControl control(target.height, target.width);
    std::uniform_real_distribution<double> r_random;
    auto r_dice = std::bind(r_random, generator);
    double max_radius = source.height > source.width ? 
      source.height : source.width;
    BoundaryChecker in_boundary {source.height, source.width};
    
    // Iterations
    for (int round = 0; round < options.iterations; ++round) {
      int updates = 0;
      for (int i = control.y_begin; i != control.y_end; 
           i += control.y_delta) {
        for (int j = control.x_begin; j != control.x_end;
             j += control.x_delta) {
          // Preparation
          Transform &transform = result(i, j);
          double &score = score_map(i, j);
          int y0 = i + transform.x;
          int x0 = j + transform.y;
          bool updated = false;
          
          // Random Search
          int dy = r_dice();
          int dx = r_dice();
          double radius = max_radius;
          while (radius > 1.0) {
            int y1 = static_cast<int>(y0 + dy * radius + 0.5);
            int x1 = static_cast<int>(x0 + dx * radius + 0.5);
            if (in_boundary(y1, x1)) {
              double new_score = Distance::Compute(target.GetPatch(i, j),
                                                   source.GetPatch(y1, x1));
              if (new_score < score) {
                score = new_score;
                transform.y = y1 - i;
                transform.x = x1 - j;
                updated = true;
              }
            }
            
            radius *= options.decay_rate;
          }
          
          // Propagation
          Transform new_transform;
          for (int k = 0; k < 2; ++k) {
            if (0 == k) {
              int new_i = i + control.y_delta;
              if (0 <= new_i && new_i < target.height) {
                new_transform = result(new_i, j);
              } else {
                continue;
              }
            } else {
              int new_j = j + control.x_delta;
              if (0 <= new_j && new_j < target.width) {
                new_transform = result(i, new_j);
              } else {
                continue;
              }
            }
            int y2 = i + new_transform.y;
            int x2 = j + new_transform.x;
            if (in_boundary(y2, x2)) {
              double new_score = Distance::Compute(target.GetPatch(i, j),
                                                   source.GetPatch(y2, x2));
              if (new_score < score) {
                score = new_score;
                transform = new_transform;
                updated= true;
              }
            }
          }

          if (updated) updates++;
        }  // for i
      }  // for j
      
      if (updates < static_cast<int>(target.height * target.width *
                                     options.termination_update_rate)) {
        break;
      }

      control.Switch();
      
    }  // for round

    return result;
  }
  
}  // namespace icon_fitter

#endif  // _ICON_FITTER_PATCHMATCH_
