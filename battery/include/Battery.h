/*
 * Copyright (C) 2022 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android-base/file.h>
#include <vendor/nameless/hardware/battery/1.0/IBattery.h>

using ::android::hardware::Return;
using ::android::hardware::Void;

using ::android::base::ReadFileToString;
using ::android::base::WriteStringToFile;

using vendor::nameless::hardware::battery::V1_0::ChargingStatus;
using vendor::nameless::hardware::battery::V1_0::Feature;

namespace vendor {
namespace nameless {
namespace hardware {
namespace battery {
namespace implementation {

struct Battery : public V1_0::IBattery {
    Return<bool> hasFeature(Feature feature) override;
    Return<ChargingStatus> readChargingStatus() override;
    Return<bool> isChargingSuspended() override;
    Return<void> setChargingSuspended(bool suspended) override;
    Return<bool> isWirelessQuietModeEnabled() override;
    Return<void> setWirelessQuietModeEnabled(bool enabled) override;
    Return<bool> isWirelessRXEnabled() override;
    Return<void> setWirelessRXEnabled(bool enabled) override;
    Return<bool> isWirelessTXEnabled() override;
    Return<void> setWirelessTXEnabled(bool enabled) override;
};

}  // namespace implementation
}  // namespace battery
}  // namespace hardware
}  // namespace nameless
}  // namespace vendor
