#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "algebra.h"
#include "feature_image.h"
#include "hog.h"
#include "patchmatch.h"

using namespace icon_fitter;

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Error: need more arguments.\n");
  }
  
  // template
  FeatureImage<float> template_image = HogGen::Create(argv[1], {6, 9, false});
  BlockFeatureImage<float> target(&template_image, 2, 6);

  // input
  FeatureImage<float> input_image = HogGen::Create(argv[2], {6, 9, false});
  BlockFeatureImage<float> source(&input_image, 2, 6);

  // PatchMatch
  PatchMatchOptions options;
  TransformMap result = PatchMatch(source, target, options);

  // Mean of Transform Map
  Transform mean;
  for (const Transform &transform : result.data()) {
    mean.y += transform.y;
    mean.x += transform.x;
  }

  mean.y /= target.height * target.width;
  mean.x /= target.height * target.width;
  
  // Visualization match result
  {
    cv::Mat input = cv::imread(argv[2]);
    cv::Mat icon = cv::imread(argv[1]);
    rectangle(input, 
              {mean.x, mean.y}, 
              {mean.x + icon.cols, mean.y + icon.rows}, 
              {0, 0, 255});
    cv::imshow("input", input);
    cv::imshow("icon", icon);
    cv::waitKey(0);
  }

  return 0;
}
