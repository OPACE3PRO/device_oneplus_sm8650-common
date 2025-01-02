/*
 * Copyright (C) 2022 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "Battery"

#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>

#include "include/Battery.h"

using vendor::nameless::hardware::battery::V1_0::IBattery;
using vendor::nameless::hardware::battery::implementation::Battery;

int main() {
    android::sp<IBattery> service = new Battery();

    android::hardware::configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (service->registerAsService() != android::OK) {
        ALOGE("Cannot register Battery HAL service");
        return 1;
    }

    ALOGI("Battery HAL service ready");

    android::hardware::joinRpcThreadpool();

    ALOGE("Battery HAL service failed to join thread pool");
    return 1;
}
