/*
 * Copyright (C) 2019 The Android Open Source Project
 * Copyright (C) 2019 STMicroelectronics
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef V4L2_CAMERA_HAL_METADATA_RANGED_CONVERTER_H_
#define V4L2_CAMERA_HAL_METADATA_RANGED_CONVERTER_H_

#include <memory>

#include "converter_interface.h"

namespace android {
namespace hardware {
namespace camera {
namespace common {
namespace V1_0 {
namespace metadata {

// An RangedConverter fits values converted by a wrapped converter
// to a stepped range (when going from metadata -> v4l2. The other
// direction remains unchanged).
template <typename TMetadata, typename TV4L2>
class RangedConverter : public ConverterInterface<TMetadata, TV4L2> {
 public:
  RangedConverter(
      std::shared_ptr<ConverterInterface<TMetadata, TV4L2>> wrapped_converter,
      TV4L2 min,
      TV4L2 max,
      TV4L2 step);

  virtual int MetadataToV4L2(TMetadata value, TV4L2* conversion) override;
  virtual int V4L2ToMetadata(TV4L2 value, TMetadata* conversion) override;

 private:
  std::shared_ptr<ConverterInterface<TMetadata, TV4L2>> wrapped_converter_;
  const TV4L2 min_;
  const TV4L2 max_;
  const TV4L2 step_;

  RangedConverter(const RangedConverter&);
  void operator=(const RangedConverter&);
};

// -----------------------------------------------------------------------------

template <typename TMetadata, typename TV4L2>
RangedConverter<TMetadata, TV4L2>::RangedConverter(
    std::shared_ptr<ConverterInterface<TMetadata, TV4L2>> wrapped_converter,
    TV4L2 min,
    TV4L2 max,
    TV4L2 step)
    : wrapped_converter_(std::move(wrapped_converter)),
      min_(min),
      max_(max),
      step_(step) {
  ALOGV("%s: enter", __FUNCTION__);
}

template <typename TMetadata, typename TV4L2>
int RangedConverter<TMetadata, TV4L2>::MetadataToV4L2(TMetadata value,
                                                      TV4L2* conversion) {
  ALOGV("%s: enter", __FUNCTION__);

  TV4L2 raw_conversion = 0;
  int res = wrapped_converter_->MetadataToV4L2(value, &raw_conversion);
  if (res) {
    ALOGE("%s: Failed to perform underlying conversion.", __FUNCTION__);
    return res;
  }

  // Round down to step (steps start at min_).
  raw_conversion -= (raw_conversion - min_) % step_;

  // Clamp to range.
  if (raw_conversion < min_) {
    raw_conversion = min_;
  } else if (raw_conversion > max_) {
    raw_conversion = max_;
  }

  *conversion = raw_conversion;
  return 0;
}

template <typename TMetadata, typename TV4L2>
int RangedConverter<TMetadata, TV4L2>::V4L2ToMetadata(TV4L2 value,
                                                      TMetadata* conversion) {
  ALOGV("%s: enter", __FUNCTION__);

  return wrapped_converter_->V4L2ToMetadata(value, conversion);
}

} // namespace metadata
} // namespace V1_0
} // namespace common
} // namespace camera
} // namespace hardware
} // namespace android

#endif  // V4L2_CAMERA_HAL_METADATA_RANGED_CONVERTER_H_
