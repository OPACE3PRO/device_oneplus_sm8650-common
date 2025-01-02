/*
 * Copyright (C) 2022 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "VibratorExt"

#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>

#include "include/VibratorExt.h"

using vendor::nameless::hardware::vibratorExt::V1_0::IVibratorExt;
using vendor::nameless::hardware::vibratorExt::implementation::VibratorExt;

int main() {
    android::sp<IVibratorExt> service = new VibratorExt();

    android::hardware::configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (service->registerAsService() != android::OK) {
        ALOGE("Cannot register VibratorExt HAL service");
        return 1;
    }

    ALOGI("VibratorExt HAL service ready");

    android::hardware::joinRpcThreadpool();

    ALOGE("VibratorExt HAL service failed to join thread pool");
    return 1;
}
