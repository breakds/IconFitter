#include "algebra.h"
#include "feature_image.h"
#include "hog.h"

using namespace icon_fitter;

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Error: need more arguments.\n");
  }

  FeatureImage<float> base = HogGen::Create(argv[1], {6, 9, false});
  BlockFeatureImage<float> hog(&base, 2, 6);

  algebra::PrintVector(hog.GetPatch(30, 30), 36);

  

  return 0;
}
