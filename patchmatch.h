#ifndef _ICON_FITTER_PATCHMATCH_
#define _ICON_FITTER_PATCHMATCH_

namespace icon_fitter {
  
  struct Transform {
    int y, x;
  };

  struct TransformMap {
    int width;
    int height;
    
    TransformMap(int width_, int height_) :
      width(width_), height(height_) {}

  private:
    std::vector<Transform> map_;
  };
  
}

#endif  // _ICON_FITTER_PATCHMATCH_
