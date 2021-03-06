/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef MODULES_PERCEPTION_TRAFFIC_LIGHT_TL_PREPROCESSOR_H_
#define MODULES_PERCEPTION_TRAFFIC_LIGHT_TL_PREPROCESSOR_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "modules/perception/lib/base/mutex.h"
#include "modules/perception/lib/base/timer.h"
#include "modules/perception/lib/config_manager/config_manager.h"
#include "modules/perception/traffic_light/base/image.h"
#include "modules/perception/traffic_light/base/image_lights.h"
#include "modules/perception/traffic_light/interface/base_preprocessor.h"
#include "modules/perception/traffic_light/projection/multi_camera_projection.h"

namespace apollo {
namespace perception {
namespace traffic_light {

using apollo::hdmap::Signal;
typedef std::vector<std::shared_ptr<LightPtrs>> LightsArray;

class TLPreprocessor : public BasePreprocessor {
 public:
  TLPreprocessor() {}
  ~TLPreprocessor() = default;

  virtual bool Init();

  virtual std::string name() const { return "TLPreprocessor"; }

  bool CacheLightsProjections(const CarPose &pose,
                              const std::vector<Signal> &signals,
                              const double ts);

  bool SyncImage(const ImageSharedPtr &image, ImageLightsPtr *image_lights,
                 bool *should_pub);

  void set_last_pub_camera_id(CameraId camera_id);
  CameraId last_pub_camera_id() const;

  int max_cached_lights_size() const;

  bool SelectCameraByProjection(const double timestamp, const CarPose &pose,
                                const std::vector<Signal> &signals,
                                std::shared_ptr<ImageLights> image_lights,
                                CameraId *selected_camera_id);

  // @brief Project lights from HDMap onto long focus or short focus image plane
  bool ProjectLights(const CarPose &pose, const std::vector<Signal> &signals,
                     const CameraId &camera_id, LightPtrs *lights_on_image,
                     LightPtrs *lights_outside_image);

 private:
  void SelectImage(const CarPose &pose,
                   const LightsArray &lights_on_image_array,
                   const LightsArray &lights_outside_image_array,
                   CameraId *selection);

  // @brief Sync. image with cached lights projections
  bool SyncImageWithCachedLights(const ImageSharedPtr &image,
                                 ImageLightsPtr *image_lights,
                                 double *diff_image_pose_ts,
                                 double *diff_image_sys_ts, bool *sync_ok);

  bool IsOnBorder(const cv::Size size, const cv::Rect &roi,
                  const int border_size) const;

  int GetMinFocalLenCameraId();
  int GetMaxFocalLenCameraId();

 private:
  MultiCamerasProjection projection_;

  double last_no_signals_ts_ = -1.0;

  CameraId last_pub_camera_id_ = CameraId::UNKNOWN;

  double last_output_ts_ = 0.0;

  std::vector<std::shared_ptr<ImageLights>> cached_lights_;

  Mutex mutex_;

  // some parameters from config file
  int max_cached_lights_size_ = 100;
  int projection_image_cols_ = 1920;
  int projection_image_rows_ = 1080;
  float sync_interval_seconds_ = 0.1;
  float no_signals_interval_seconds_ = 0.5;

  DISALLOW_COPY_AND_ASSIGN(TLPreprocessor);
};
}  // namespace traffic_light
}  // namespace perception
}  // namespace apollo

#endif  // MODULES_PERCEPTION_TRAFFIC_LIGHT_TL_PREPROCESSOR_H_
