/*
 * Copyright (C) 2022 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "Battery"

#include <log/log.h>

#include "include/Battery.h"

namespace vendor {
namespace nameless {
namespace hardware {
namespace battery {
namespace implementation {

static const char CHARGING_SUSPEND_PATH[]    = "/sys/class/oplus_chg/battery/mmi_charging_enable";
static const char VOOCCHG_ING_PATH[]         = "/sys/class/oplus_chg/battery/voocchg_ing";
static const char WIRELESS_QUIET_MODE_PATH[] = "/proc/wireless/user_sleep_mode";
static const char WIRELESS_RX_PATH[]         = "/proc/wireless/enable_rx";
static const char WIRELESS_TX_PATH[]         = "/proc/wireless/enable_tx";

const uint32_t SUPPORTED_FEATURES = Feature::SUSPEND_CHARGING |
                                    Feature::WIRELESS_CHARGING_QUIET_MODE |
                                    Feature::WIRELESS_CHARGING_RX |
                                    Feature::WIRELESS_CHARGING_TX;

Return<bool> Battery::hasFeature(Feature feature) {
    return (feature & SUPPORTED_FEATURES) != 0;
}

Return<ChargingStatus> Battery::readChargingStatus() {
    std::string value;
    ReadFileToString(VOOCCHG_ING_PATH, &value);
    return value != "0\n" ? ChargingStatus::VOOC_CHARGING : ChargingStatus::UNKNOWN;
}

Return<bool> Battery::isChargingSuspended() {
    std::string value;
    return ReadFileToString(CHARGING_SUSPEND_PATH, &value) && value != "1\n";
}

Return<void> Battery::setChargingSuspended(bool suspended) {
    if (!WriteStringToFile(suspended ? "0" : "1", CHARGING_SUSPEND_PATH, true)) {
        ALOGE("Failed to write charging suspended file");
    }
    return Void();
}

Return<bool> Battery::isWirelessQuietModeEnabled() {
    std::string value;
    return ReadFileToString(WIRELESS_QUIET_MODE_PATH, &value) && value != "0\n";
}

Return<void> Battery::setWirelessQuietModeEnabled(bool enabled) {
    if (!WriteStringToFile(enabled ? "1" : "0", WIRELESS_QUIET_MODE_PATH, true)) {
        ALOGE("Failed to write wireless quiet mode file");
    }
    return Void();
}

Return<bool> Battery::isWirelessRXEnabled() {
    std::string value;
    return ReadFileToString(WIRELESS_RX_PATH, &value) && value != "0\n";
}

Return<void> Battery::setWirelessRXEnabled(bool enabled) {
    if (!WriteStringToFile(enabled ? "1" : "0", WIRELESS_RX_PATH, true)) {
        ALOGE("Failed to write wireless RX file");
    }
    return Void();
}

Return<bool> Battery::isWirelessTXEnabled() {
    std::string value;
    return ReadFileToString(WIRELESS_TX_PATH, &value) && value != "disable\n";
}

Return<void> Battery::setWirelessTXEnabled(bool enabled) {
    if (!WriteStringToFile(enabled ? "1" : "0", WIRELESS_TX_PATH, true)) {
        ALOGE("Failed to write wireless TX file");
    }
    return Void();
}

}  // namespace implementation
}  // namespace battery
}  // namespace hardware
}  // namespace nameless
}  // namespace vendor
