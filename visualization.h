#ifndef _ICON_FITTER_VISUALIZATION_
#define _ICON_FITTER_PATCHMATCH_

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "patchmatch.h"

namespace icon_fitter {

  class TransformViewer {
  public:
    TransformViewer(const std::string &source_path,
                    const std::string &target_path,
                    const TransformMap *transform_map,
                    int cell_size)
      : source_(cv::imread(source_path)),
        target_(cv::imread(target_path)),
        cell_size_(cell_size),
        map_(transform_map),
        target_y_(-1),
        target_x_(-1) {
      Refresh();
    }

    void Refresh() {
      if (target_y_ < 0 || target_x_ < 0) {
        cv::imshow("source", source_);
        cv::imshow("target", target_);
        cv::setMouseCallback("target", TargetOnClick, this);
      } else {
        source_copy_ = source_.clone();
        target_copy_ = target_.clone();
        int y = static_cast<int>(target_y_ - cell_size_ * 0.5);
        int x = static_cast<int>(target_x_ - cell_size_ * 0.5);
        if (y < 0) y = 0;
        if (x < 0) x = 0;
        cv::rectangle(target_copy_, 
                      {x, y},
                      {x + cell_size_, y + cell_size_},
                      {0, 255, 0});
        const Transform &transform = map_->Get(y, x);
        y = y + transform.y;
        x = x + transform.x;
        cv::rectangle(source_copy_, 
                      {x, y},
                      {x + cell_size_, y + cell_size_},
                      {0, 255, 0});
        cv::imshow("source", source_copy_);
        cv::imshow("target", target_copy_);
        cv::setMouseCallback("target", TargetOnClick, this);
      }
      cv::waitKey(0);
    }
    
  private:
    
    static void TargetOnClick(int event, int x, int y, int flag, void* user_data) {
      if (event != cv::EVENT_LBUTTONDOWN) return;
      TransformViewer *viewer = static_cast<TransformViewer*>(user_data);
      viewer->target_y_ = y;
      viewer->target_x_ = x;
      viewer->Refresh();
    }
    
    cv::Mat source_;
    cv::Mat source_copy_;
    cv::Mat target_;
    cv::Mat target_copy_;
    int cell_size_;
    const TransformMap *map_;
    int target_y_;
    int target_x_;
  };

}  // namespace icon_fitter

#endif  // _ICON_FITTER_VISUALIZATION_
