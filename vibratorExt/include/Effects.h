/*
 * Copyright (C) 2022-2024 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <log/log.h>
#include <unordered_map>
#include <vector>
#include <vendor/nameless/hardware/vibratorExt/1.0/IVibratorExt.h>

using std::unordered_map;
using std::vector;

using vendor::nameless::hardware::vibratorExt::V1_0::Effect;
using vendor::nameless::hardware::vibratorExt::V1_0::Style;

#define DEBUG                     false

const uint32_t kFirmwareIdMax   = 999;
const uint32_t kPlayRateHz      = 24000;
const uint32_t kEffectInterval  = 50;
const uint32_t kStyleFollowUser = 100;

struct effect_stream {
    uint32_t         effect_id;
    uint32_t         length;
    uint32_t         play_rate_hz = kPlayRateHz;
    const int8_t     *data;
};

struct custom_effect {
    vector<uint32_t> firmware_id;
    vector<uint32_t> sleep_time;
    vector<float>    scale;
    bool             fixed_scale = false;
    uint32_t         min_interval = kEffectInterval;
    uint32_t         style = kStyleFollowUser;
};

struct custom_effect_style_set {
    custom_effect    crisp;
    custom_effect    gentle;
};

bool hasEffect(Effect effectId);
Effect durationToEffectId(uint64_t duration);
custom_effect *getEffect(Effect effectId, Style style);
effect_stream *loadFirmwareEffect(uint32_t effectId, Style style);
