#ifndef _ICON_FITTER_HOG_
#define _ICON_FITTER_HOG_

#include <cstdio>
#include <cmath>
#include <utility>

#define _USE_MATH_DEFINES

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "feature_image.h"

namespace icon_fitter {

  struct HogOptions {
    // Cell parameters
    int cell_size;
    // Feature parameters
    int bins;
    bool signed_orientation;
  };

  struct OrientationBucketer {
    double bin_size;
    bool is_signed;

    OrientationBucketer(const HogOptions &options) 
      : bin_size(options.signed_orientation ? 
                 M_PI * 2 / options.bins :
                 M_PI / options.bins),
        is_signed(options.signed_orientation) {}
      
    inline std::pair<int, double> operator()(double y, double x) {
      double position;
      if (is_signed) {
        position = fabs(atan2(y, x) + M_PI);
      } else {
        double angle = atan2(y, x);
        if (angle < 0.0) {
          position = fabs(angle + M_PI);
        } else {
          position = angle;
        }
      }
      return std::make_pair(static_cast<int>(position / bin_size),
                            Magnitude(y, x));
    }
      
  private:
    inline static double Magnitude(double a, double b) {
      return sqrt(a * a + b * b);
    }
  };


   
  struct HogGen {
    static FeatureImage<float> Create(const cv::Mat &input,
                                      HogOptions options) {
      cv::Mat processed;

      // Grayscalization
      cv::cvtColor(input, processed, cv::COLOR_BGR2GRAY);

      // Normalization
      cv::normalize(processed, processed, 0, 255, cv::NORM_MINMAX);
      
      // Gradient
      cv::Mat gradx(input.rows, input.cols, CV_32FC1);
      cv::Mat grady(input.rows, input.cols, CV_32FC1);
      
      cv::Sobel(processed, gradx, CV_32F, 1, 0);
      cv::Sobel(processed, grady, CV_32F, 0, 1);
    
    
      // Histogram voting
      FeatureImage<float> result(input.rows, input.cols, options.bins);
      OrientationBucketer bucketer(options);
      double half_cell_size = options.cell_size * 0.5;
      
      for (int i = 0; i < input.rows; ++i) {
        float *gxptr = gradx.ptr<float>(i);
        float *gyptr = grady.ptr<float>(i);
        for (int j = 0; j < input.cols; ++j) {
          auto vote = bucketer(*(gyptr++), *(gxptr++));
          for (int y = i - options.cell_size + 1; y <= i; ++y) {
            if (0 > y) continue;
            for (int x = j - options.cell_size + 1; x <= j; ++x) {
              if (0 > x) continue;
              float *feature = result.mutable_feature(y, x);
              double weight_y = fabs(i - y - half_cell_size) / options.cell_size;
              double weight_x = fabs(j - x - half_cell_size) / options.cell_size;
              feature[vote.first] += (1.0 - weight_x) * (1.0 - weight_y) * vote.second;
            }
          }
        }
      }
      result.Normalize();
      return result;
    }

    static FeatureImage<float> Create(const std::string &filename, 
                                      HogOptions options) {
      cv::Mat input = cv::imread(filename);
      if (input.empty()) {
        printf("Error: Failed to read image %s\n", filename.c_str());
        exit(-1);
      }
      return Create(input, options);
    }
    
    
  };






}  // namespace icon_fitter
                               


#endif  // _ICON_FITTER_HOG_

