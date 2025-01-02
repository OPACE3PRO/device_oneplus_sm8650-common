/*
 * Copyright (C) 2022-2024 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android-base/file.h>
#include <dirent.h>
#include <inttypes.h>
#include <linux/input.h>

#include "include/Effects.h"

using ::android::hardware::Return;
using ::android::hardware::Void;

using vendor::nameless::hardware::vibratorExt::V1_0::LevelRange;
using vendor::nameless::hardware::vibratorExt::V1_0::Type;

namespace vendor {
namespace nameless {
namespace hardware {
namespace vibratorExt {
namespace implementation {

struct VibratorExt : public V1_0::IVibratorExt {
    Return<void> initVibrator() override;
    Return<int64_t> vibratorOn(Effect effectId, uint64_t duration) override;
    Return<void> vibratorOff() override;
    Return<void> setAmplitude(float amplitude) override;
    Return<void> setHapticStyle(Style style) override;
    Return<void> getStrengthLevelRange(Type type, getStrengthLevelRange_cb callback) override;
    Return<void> setStrengthLevel(Type type, uint32_t level) override;
    Return<bool> isEffectSupported(Effect effectId) override;
    Return<bool> isHapticStyleSupported(Style style) override;
};

}  // namespace implementation
}  // namespace vibratorExt
}  // namespace hardware
}  // namespace nameless
}  // namespace vendor
