#ifndef _ICON_FITTER_FEATURE_IMAGE_
#define _ICON_FITTER_FEATURE_IMAGE_

#include <vector>

#include "algebra.h"


namespace icon_fitter {

  template <typename DataType>
  struct FeatureImage {
  public:
    int width;
    int height;
    int depth;

    FeatureImage(int height_, int width_, int depth_)
      : height(height_), width(width_), depth(depth_),
        data_(height * width * depth, 0.0) {}

    FeatureImage(FeatureImage &&other) {
      width = other.width;
      height = other.height;
      depth = other.depth;
      data_ = std::move(other.data_);
    }

    const FeatureImage &operator=(FeatureImage &&other) {
      width = other.width;
      height = other.height;
      depth = other.depth;
      data_ = std::move(other.data_);
    }
    
    inline const DataType *feature(int y, int x) const {
      return &data_[(y * width + x) * depth];
    }

    inline const DataType *feature(int id) const {
      return &data_[id * depth];
    }

    inline DataType *mutable_feature(int y, int x) {
      return &data_[(y * width + x) * depth];
    }

    inline DataType *mutable_feature(int id) {
      return &data_[id * depth];
    }

    inline int size() const {
      return width * height;
    }

    inline void Normalize() {
      for (int i = 0; i < height * width; ++i) {
        algebra::Normalize(mutable_feature(i), depth);
      }
    }

  private:
    std::vector<DataType> data_;
  };


  // ---------- Blocked Featuer Image ----------

  template <typename DataType>
  struct BlockFeatureImage;

  template <typename DataType>
  class Patch {
    friend class BlockFeatureImage<DataType>;
    
  public:
    Patch(const BlockFeatureImage<DataType> *parent,
          const DataType *begin) 
      : parent_(parent), begin_(begin) {}

    Patch(const Patch<DataType> &other) = default;

    inline DataType operator[](int index) const {
      return *(begin_ + parent_->offsets_[index]);
    }

    inline int size() {
      return parent_->dimension_;
    }
    
  private:
    const BlockFeatureImage<DataType> *parent_;
    const DataType *begin_;
  };
    


  template <typename DataType>
  struct BlockFeatureImage {
    friend class Patch<DataType>;

    int block_size;
    int stride;
    
    BlockFeatureImage(const FeatureImage<DataType> *image, 
                      int block_size_, 
                      int stride_) 
      : block_size(block_size_), stride(stride_), image_(image) {
      dimension_ = block_size * block_size * image->depth;
      offsets_.resize(dimension_);
      int id = 0;
      for (int i = 0; i < block_size; ++i) {
        for (int j = 0; j < block_size; ++j) {
          int base = (i * stride * image->width + j * stride) * image->depth;
          for (int k = 0; k < image->depth; ++k) {
            offsets_[id++] = (base++);
          }
        }
      }
      
      // Create patches
      patches_.reserve(image_->size());
      for (int i = 0; i < image_->size(); ++i) {
        patches_.emplace_back(this, image_->feature(i));
      }
    }

    inline const Patch<DataType> &GetPatch(int i, int j) const {
      return patches_[i * image_->width + j];
    }

    inline const Patch<DataType> &GetPatch(int id) const {
      return patches_[id];
    }

  private:
    int dimension_;
    const FeatureImage<DataType> *image_;
    std::vector<int> offsets_;
    std::vector<Patch<DataType> > patches_;
  };

}  // namespace icon_fitter


#endif  // _ICON_FITTER_FEATURE_IMAGE_
