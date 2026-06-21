// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_CAMERA_CAMERA_HANDLE_H_
#define COMET_RUNTIME_CAMERA_CAMERA_HANDLE_H_

#include "comet/core/essentials.h"
#include "comet/core/handle/handle.h"

namespace comet {
namespace camera {
struct CameraTag {};
using CameraHandle = Handle<CameraTag>;
}  // namespace camera
}  // namespace comet

#endif  // COMET_RUNTIME_CAMERA_CAMERA_HANDLE_H_