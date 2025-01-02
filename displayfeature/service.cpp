/*
 * Copyright (C) 2022 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "DisplayFeature"

#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>

#include "include/DisplayFeature.h"

using vendor::nameless::hardware::displayfeature::V1_0::IDisplayFeature;
using vendor::nameless::hardware::displayfeature::implementation::DisplayFeature;

int main() {
    android::sp<IDisplayFeature> service = new DisplayFeature();

    android::hardware::configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (service->registerAsService() != android::OK) {
        ALOGE("Cannot register DisplayFeature HAL service");
        return 1;
    }

    ALOGI("DisplayFeature HAL service ready");

    android::hardware::joinRpcThreadpool();

    ALOGE("DisplayFeature HAL service failed to join thread pool");
    return 1;
}
