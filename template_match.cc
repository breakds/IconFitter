#include <cstdio>
#include <string>

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

class TemplatePyramid {
public:
  static const int MIN_DIMENSION;
  TemplatePyramid(const cv::Mat &input, double shrink_rate) 
    : templates_() {
    cv::Mat current = input;
    while (MIN_DIMENSION <= current.rows &&
           MIN_DIMENSION <= current.cols) {
      cv::GaussianBlur(current, current, cv::Size(5, 5), 1.2, 1.2);
      templates_.push_back(GetGradient(current));
      cv::resize(current, current, 
                 cv::Size(current.cols * shrink_rate,
                          current.rows * shrink_rate));
    }
  }

  TemplatePyramid(const std::string &path, double shrink_rate) 
    : TemplatePyramid(cv::imread(path), shrink_rate) {}
  
  int layers() const {
    return templates_.size();
  }
  
  const cv::Mat &layer(int id) const {
    return templates_[id];
  }

private:
  
  cv::Mat GetGradient(const cv::Mat &input) {
    cv::Mat result;
    cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
    cv::Mat edges(input.rows, input.cols, CV_32FC1);
    cv::Canny(result, edges, 100.0, 300.0, 5);
    return edges;
  }
  
  std::vector<cv::Mat> templates_;
};

const int TemplatePyramid::MIN_DIMENSION = 20;

TemplatePyramid LoadTemplate(const char *path) {
  cv::Mat raw = cv::imread(path);
  TemplatePyramid templates(raw, 0.8);
  // for (int i = 0; i < templates.layers(); ++i) {
  //   cv::imwrite("output/template_" + std::to_string(i) + ".png",
  //               templates.layer(i));
  // }
  return templates;
}

cv::Mat LoadTarget(const char *path) {
  cv::Mat raw = cv::imread(path);
  cv::Mat normalized;
  cv::GaussianBlur(raw, normalized, cv::Size(5, 5), 1.2, 1.2);
  cv::Mat gray;
  cv::cvtColor(raw, gray, cv::COLOR_BGR2GRAY);
  cv::Mat edges(raw.rows, raw.cols, CV_32FC1);
  cv::Canny(gray, edges, 100.0, 300.0, 5);
  return edges;
}

void GenerateMatch(const TemplatePyramid &templates,
                   const cv::Mat &target_img) {
  for (int i = 0; i < templates.layers(); ++i) {
    if (templates.layer(i).rows <= target_img.rows * 0.5 &&
        templates.layer(i).cols <= target_img.cols * 0.5) {
      cv::Mat result;
      cv::matchTemplate(target_img, templates.layer(i), 
                        result, cv::TM_SQDIFF);
      cv::Point max_loc, min_loc;
      double max_value, min_value;
      cv::minMaxLoc(result, 
                    &min_value, &max_value,
                    &min_loc, &max_loc);
      cv::Mat output;
      cv::cvtColor(target_img, output, cv::COLOR_GRAY2BGR);
      cv::rectangle(output, max_loc, 
                    cv::Point(max_loc.x + templates.layer(i).cols,
                              max_loc.y + templates.layer(i).rows),
                    cv::Scalar(0, 255, 0));
      cv::imwrite("output/generate_" + std::to_string(i) + ".png",
                  output);
    }
  }
}

int main(int argc, char **argv) {
  GenerateMatch(TemplatePyramid(argv[1], 0.8),
                LoadTarget(argv[2]));
  return 0;
}

