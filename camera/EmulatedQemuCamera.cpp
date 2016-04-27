/*
 * Copyright (C) 2011 The Android Open Source Project
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

/*
 * Contains implementation of a class EmulatedQemuCamera that encapsulates
 * functionality of an emulated camera connected to the host.
 */

#define LOG_NDEBUG 0
#define LOG_TAG "EmulatedCamera_QemuCamera"
#include <cutils/log.h>
#include <cutils/properties.h>
#include "EmulatedQemuCamera.h"
#include "EmulatedCameraFactory.h"

namespace android {

EmulatedQemuCamera::EmulatedQemuCamera(int cameraId, struct hw_module_t* module)
        : EmulatedCamera(cameraId, module),
          mQemuCameraDevice(this)
{
}

EmulatedQemuCamera::~EmulatedQemuCamera()
{
}

static int getCameraOrientation()
{
    char prop[PROPERTY_VALUE_MAX];
    int height, width, depth, orientation = 0;
    if ((property_get("aicVM.vbox_graph_mode", prop, NULL)) && sscanf(prop, "%dx%d-%d", &width, &height, &depth) > 3) {
        if (height > width)
            orientation = 90;
    }
    return orientation;

}

/****************************************************************************
 * EmulatedCamera virtual overrides.
 ****************************************************************************/

status_t EmulatedQemuCamera::Initialize(const char* device_name,
                                        const int port)
{
#if 0
    ALOGV("%s:\n   Name=%s\n   Facing '%s'\n   Dimensions=%s",
         __FUNCTION__, device_name, facing_dir, frame_dims);
#endif
    /* Save dimensions. */
    const char frame_dims[] = "640x480,320x240,176x144";
    mFrameDims = String8(frame_dims);
    const char preview_frame_dims[] = "320x240,176x144";

    /* Initialize camera device. */
    status_t res = mQemuCameraDevice.Initialize(device_name, port);
    if (res != NO_ERROR) {
        return res;
    }

    /* Initialize base class. */
    res = EmulatedCamera::Initialize();
    if (res != NO_ERROR) {
        return res;
    }

    /*
     * Set customizable parameters.
     */

    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_FRAME_RATES,
                    "5,10,15,20,25,30");

    mParameters.set(EmulatedCamera::FACING_KEY,
                    (strcmp(device_name, EmulatedCamera::FACING_FRONT) == 0) ?
                    EmulatedCamera::FACING_FRONT :
                    EmulatedCamera::FACING_BACK);
    mParameters.set(EmulatedCamera::ORIENTATION_KEY, getCameraOrientation());
    mParameters.set(CameraParameters::KEY_PREVIEW_FRAME_RATE, "24");
    mParameters.set(CameraParameters::KEY_PREVIEW_FPS_RANGE, "5000,30000");
    mParameters.set(CameraParameters::KEY_SUPPORTED_PICTURE_SIZES, frame_dims);
    mParameters.set(CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, frame_dims);
    mParameters.set(CameraParameters::KEY_SUPPORTED_PREVIEW_SIZES, preview_frame_dims);
    mParameters.set(CameraParameters::KEY_PREFERRED_PREVIEW_SIZE_FOR_VIDEO, "320x240");

    /*
     * Use first dimension reported by the device to set current preview and
     * picture sizes.
     */
#if 0

    char first_dim[128];
    /* Dimensions are separated with ',' */
    const char* c = strchr(frame_dims, ',');
    if (c == NULL) {
        strncpy(first_dim, frame_dims, sizeof(first_dim));
        first_dim[sizeof(first_dim)-1] = '\0';
    } else if (static_cast<size_t>(c - frame_dims) < sizeof(first_dim)) {
        memcpy(first_dim, frame_dims, c - frame_dims);
        first_dim[c - frame_dims] = '\0';
    } else {
        memcpy(first_dim, frame_dims, sizeof(first_dim));
        first_dim[sizeof(first_dim)-1] = '\0';
    }

    /* Width and height are separated with 'x' */
    char* sep = strchr(first_dim, 'x');
    if (sep == NULL) {
        ALOGE("%s: Invalid first dimension format in %s",
             __FUNCTION__, frame_dims);
        return EINVAL;
    }

    *sep = '\0';
    const int x = atoi(first_dim);
    const int y = atoi(sep + 1);
    mParameters.setPreviewSize(x, y);
    mParameters.setPictureSize(x, y);
#endif
    int x= 640, y = 480;
    mParameters.setPreviewSize(x, y);
    mParameters.setPictureSize(x, y);

    ALOGV("%s: Qemu camera %s is initialized. Current frame is %dx%d",
         __FUNCTION__, device_name, x, y);

    return NO_ERROR;
}

EmulatedCameraDevice* EmulatedQemuCamera::getCameraDevice()
{
    return &mQemuCameraDevice;
}

};  /* namespace android */
