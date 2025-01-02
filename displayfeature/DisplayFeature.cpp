/*
 * Copyright (C) 2022 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "DisplayFeature"
#define DEBUG true

#include <log/log.h>

#include "include/DisplayFeature.h"

namespace vendor {
namespace nameless {
namespace hardware {
namespace displayfeature {
namespace implementation {

static const char MIN_FPS_PATH[] = "/sys/kernel/oplus_display/min_fps";
static const char SEED_PATH[]    = "/sys/kernel/oplus_display/seed";
static const char TEST_TE_PATH[] = "/sys/kernel/oplus_display/test_te";

uint32_t SUPPORTED_FEATURES = Feature::HIGH_SAMPLE_TOUCH |
                              Feature::LTPO;

std::shared_ptr<IOplusTouch> mTouchService = nullptr;

static std::string touchReadNodeFile(int featureId) {
    if (mTouchService == nullptr) {
        const std::string instance = std::string() + IOplusTouch::descriptor + "/default";
        mTouchService = IOplusTouch::fromBinder(
            ndk::SpAIBinder(AServiceManager_waitForService(instance.c_str())));
    }
    if (mTouchService != nullptr) {
        std::string result;
        mTouchService->touchReadNodeFile(0, featureId, &result);
        if (DEBUG) {
            ALOGD("touchReadNodeFile, featureId=%d, result=%s", featureId, result.c_str());
        }
        return result;
    }
    return "";
}

static void touchWriteNodeFile(int featureId, std::string value) {
    if (mTouchService == nullptr) {
        const std::string instance = std::string() + IOplusTouch::descriptor + "/default";
        mTouchService = IOplusTouch::fromBinder(
            ndk::SpAIBinder(AServiceManager_waitForService(instance.c_str())));
    }
    if (mTouchService != nullptr) {
        int result;
        mTouchService->touchWriteNodeFile(0, featureId, value, &result);
        if (DEBUG) {
            ALOGD("touchWriteNodeFile, featureId=%d, value=%s, result=%d", featureId, value.c_str(), result);
        }
    }
}

Return<bool> DisplayFeature::hasFeature(Feature feature) {
    return (feature & SUPPORTED_FEATURES) != 0;
}

Return<bool> DisplayFeature::getFeatureEnabled(Feature feature) {
    if (!hasFeature(feature)) {
        ALOGE("Feature id %d is unsupported", static_cast<int>(feature));
        return false;
    }
    std::string value;
    switch (feature) {
        case Feature::HIGH_SAMPLE_TOUCH:
            return touchReadNodeFile(26).find("3") != std::string::npos;
        case Feature::LTPO:
            return ReadFileToString(MIN_FPS_PATH, &value) && value == "1";
        default:
            ALOGE("Unknown feature id %d", static_cast<int>(feature));
            break;
    }
    return false;
}

Return<void> DisplayFeature::setFeatureEnabled(Feature feature, bool enabled) {
    if (!hasFeature(feature)) {
        ALOGE("Feature id %d is unsupported", static_cast<int>(feature));
        return Void();
    }
    std::string value;
    switch (feature) {
        case Feature::HIGH_SAMPLE_TOUCH:
            touchWriteNodeFile(26, enabled ? "3" : "0");
            break;
        case Feature::LTPO:
            if (!WriteStringToFile(enabled ? "1" : "120", MIN_FPS_PATH, true) ||
                    !WriteStringToFile(enabled ? "1" : "0", TEST_TE_PATH, true)) {
                ALOGE("Failed to write min fps file");
            }
            break;
        default:
            ALOGE("Unknown feature id %d", static_cast<int>(feature));
            break;
    }
    return Void();
}

Return<void> DisplayFeature::setColorMode(uint32_t mode) {
    switch (mode) {
        case 303:  // Natural
            WriteStringToFile("102", SEED_PATH, true);
            break;
        case 301:  // Professional
            WriteStringToFile("101", SEED_PATH, true);
            break;
        case 307:  // Saturated
            WriteStringToFile("100", SEED_PATH, true);
            break;
        default:
            break;
    }
    return Void();
}

Return<void> DisplayFeature::setDisplayOrientation(uint32_t orientation) {
    return Void();
}

Return<int32_t> DisplayFeature::sendCommand(Command command) {
    std::string value;
    switch (command) {
        case Command::CMD_INIT_TOUCH_GESTURE:
            touchWriteNodeFile(1, "1");
            touchWriteNodeFile(21, "78068");
            return 0;
        case Command::CMD_REPORT_TOUCH_GESTURE:
            value = touchReadNodeFile(2);
            return std::stoi(value.substr(0, value.find(",")));
        default:
            return -1;
    }
}

Return<uint32_t> DisplayFeature::getCurrentFps() {
    std::string value;
    ReadFileToString(MIN_FPS_PATH, &value);
    if (value != "1") {
        return 0;
    }
    ReadFileToString(TEST_TE_PATH, &value);
    return std::stoi(value);
}

}  // namespace implementation
}  // namespace displayfeature
}  // namespace hardware
}  // namespace nameless
}  // namespace vendor
