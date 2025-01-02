/*
 * Copyright (C) 2022 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/oplus/hardware/touch/IOplusTouch.h>
#include <android-base/file.h>
#include <android-base/properties.h>
#include <android/binder_manager.h>
#include <vendor/nameless/hardware/displayfeature/1.0/IDisplayFeature.h>

using ::aidl::vendor::oplus::hardware::touch::IOplusTouch;

using ::android::hardware::Return;
using ::android::hardware::Void;

using ::android::base::GetProperty;
using ::android::base::ReadFileToString;
using ::android::base::WriteStringToFile;

using vendor::nameless::hardware::displayfeature::V1_0::Command;
using vendor::nameless::hardware::displayfeature::V1_0::Feature;

namespace vendor {
namespace nameless {
namespace hardware {
namespace displayfeature {
namespace implementation {

struct DisplayFeature : public V1_0::IDisplayFeature {
    Return<bool> hasFeature(Feature feature) override;
    Return<bool> getFeatureEnabled(Feature feature) override;
    Return<void> setFeatureEnabled(Feature feature, bool enabled) override;
    Return<void> setColorMode(uint32_t mode) override;
    Return<void> setDisplayOrientation(uint32_t orientation) override;
    Return<int32_t> sendCommand(Command command) override;
    Return<uint32_t> getCurrentFps() override;
};

}  // namespace implementation
}  // namespace displayfeature
}  // namespace hardware
}  // namespace nameless
}  // namespace vendor
