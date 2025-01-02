/*
 * Copyright (C) 2022-2024 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <filesystem>
#include <fstream>
#include <include/Effects.h>
#include <iostream>

const std::string kFirmwarePathPrefixDef  = "/odm/etc/vibrator/def/effect_";
const std::string kFirmwarePathPrefixSoft = "/odm/etc/vibrator/soft/effect_";

static unordered_map<Style, unordered_map<uint32_t, effect_stream>> sFirmwareData;
static unordered_map<Effect, custom_effect_style_set> sEffectMap {
    // AOSP effects
    { Effect::CLICK, {
        .crisp = {
            .firmware_id = { 6 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.8 },
            .min_interval = 80
        }
    }},
    { Effect::DOUBLE_CLICK, {
        .crisp = {
            .firmware_id = { 8, 8 },
            .sleep_time = { 150 }
        },
        .gentle = {
            .firmware_id = { 6, 6 },
            .sleep_time = { 150 }
        }
    }},
    { Effect::TICK, {
        .crisp = {
            .firmware_id = { 2 },
            .min_interval = 120
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.6 },
            .min_interval = 120
        }
    }},
    { Effect::THUD, {
        .crisp = {
            .firmware_id = { 4 }
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.8 }
        }
    }},
    { Effect::POP, {
        .crisp = {
            .firmware_id = { 4 }
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.8 }
        }
    }},
    { Effect::HEAVY_CLICK, {
        .crisp = {
            .firmware_id = { 8 }
        },
        .gentle = {
            .firmware_id = { 6 }
        }
    }},
    // Custom effects
    { Effect::DURATION_STRENGTH_LEVEL1, {
        .crisp = {
            .firmware_id = { 110 },
            .scale = { 0.6 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.18 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL2, {
        .crisp = {
            .firmware_id = { 110 },
            .scale = { 0.7 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.26 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL3, {
        .crisp = {
            .firmware_id = { 110 },
            .scale = { 0.8 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.34 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL4, {
        .crisp = {
            .firmware_id = { 111 },
            .scale = { 0.8 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.42 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL5, {
        .crisp = {
            .firmware_id = { 111 },
            .scale = { 0.9 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.50 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL6, {
        .crisp = {
            .firmware_id = { 111 },
            .scale = { 1.0 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.68 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL7, {
        .crisp = {
            .firmware_id = { 8 },
            .scale = { 0.7 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.76 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL8, {
        .crisp = {
            .firmware_id = { 8 },
            .scale = { 0.8 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.84 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL9, {
        .crisp = {
            .firmware_id = { 8 },
            .scale = { 0.9 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.92 },
            .min_interval = 80
        }
    }},
    { Effect::DURATION_STRENGTH_LEVEL10, {
        .crisp = {
            .firmware_id = { 8 },
            .scale = { 1.0 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 1.0 },
            .min_interval = 80
        }
    }},
    { Effect::RINGTONE_CUT, {
        .crisp = {
            .firmware_id = { 42 },
            .scale = { 0.8 }
        },
        .gentle = {
            .firmware_id = { 42 },
            .scale = { 0.8 },
            .style = static_cast<uint32_t>(Style::CRISP)
        }
    }},
    { Effect::ALERT_SLIDER_BOTTOM, {
        .crisp = {
            .firmware_id = { 308 },
            .scale = { 0.8 }
        },
        .gentle = {
            .firmware_id = { 308 },
            .scale = { 0.8 },
            .style = static_cast<uint32_t>(Style::CRISP)
        }
    }},
    { Effect::ALERT_SLIDER_MIDDLE, {
        .crisp = {
            .firmware_id = { 365 }
        },
        .gentle = {
            .firmware_id = { 365 },
            .style = static_cast<uint32_t>(Style::CRISP)
        }
    }},
    { Effect::BACK_GESTURE, {
        .crisp = {
            .firmware_id = { 4 }
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.8 }
        }
    }},
    { Effect::BUTTON_CLICK, {
        .crisp = {
            .firmware_id = { 4 }
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.8 }
        }
    }},
    { Effect::CLEAR_ALL_NOTIFICATION, {
        .crisp = {
            .firmware_id = { 4, 4, 4, 4, 4 },
            .sleep_time = { 30, 30, 30, 30 }
        },
        .gentle = {
            .firmware_id = { 6, 6, 6, 6, 6 },
            .sleep_time = { 30, 30, 30, 30 },
            .scale = { 0.6, 0.6, 0.6, 0.6, 0.6 }
        }
    }},
    { Effect::CLEAR_ALL_RECENT, {
        .crisp = {
            .firmware_id = { 47 }
        },
        .gentle = {
            .firmware_id = { 47 },
            .style = static_cast<uint32_t>(Style::CRISP)
        }
    }},
    { Effect::KEYBOARD_PRESS, {
        .crisp = {
            .firmware_id = { 8, 8 },
            .sleep_time = { 100 },
            .min_interval = 80
        },
        .gentle = {
            .firmware_id = { 6, 6 },
            .sleep_time = { 100 },
            .min_interval = 80
        }
    }},
    { Effect::PLUG_IN, {
        .crisp = {
            .firmware_id = { kFirmwareIdMax + 200, 108 },
            .sleep_time = { 600 }
        },
        .gentle = {
            .firmware_id = { kFirmwareIdMax + 200, 108 },
            .sleep_time = { 600 },
            .style = static_cast<uint32_t>(Style::CRISP)
        }
    }},
    { Effect::SCREEN_OFF, {
        .crisp = {
            .firmware_id = { 315 }
        },
        .gentle = {
            .firmware_id = { 315 },
            .style = static_cast<uint32_t>(Style::CRISP)
        }
    }},
    { Effect::SCREEN_ON, {
        .crisp = {
            .firmware_id = { 2 }
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.6 }
        }
    }},
    { Effect::SCREENSHOT, {
        .crisp = {
            .firmware_id = { 47 }
        },
        .gentle = {
            .firmware_id = { 47 },
            .style = static_cast<uint32_t>(Style::CRISP)
        }
    }},
    { Effect::SLIDER_EDGE, {
        .crisp = {
            .firmware_id = { 8 }
        },
        .gentle = {
            .firmware_id = { 6 }
        }
    }},
    { Effect::SLIDER_STEP, {
        .crisp = {
            .firmware_id = { 111 },
            .min_interval = 120
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.6 },
            .min_interval = 120
        }
    }},
    { Effect::SWITCH_TOGGLE, {
        .crisp = {
            .firmware_id = { 4 }
        },
        .gentle = {
            .firmware_id = { 6 },
            .scale = { 0.8 }
        }
    }},
    { Effect::UNIFIED_ERROR, {
        .crisp = {
            .firmware_id = { 46 }
        },
        .gentle = {
            .firmware_id = { 6, 6 },
            .sleep_time = { 120 }
        }
    }},
    { Effect::UNIFIED_SUCCESS, {
        .crisp = {
            .firmware_id = { 109 }
        },
        .gentle = {
            .firmware_id = { 6 }
        }
    }}
};

bool isHapticFeedback(Effect effectId) {
    if (effectId < Effect::DURATION_DEFAULT) {
        return true;
    }
    if (effectId >= Effect::DURATION_STRENGTH_LEVEL1 &&
            effectId <= Effect::DURATION_STRENGTH_LEVEL10) {
        return true;
    }
    if (effectId > Effect::CUSTOM_EFFECT_START &&
            effectId < Effect::CUSTOM_EFFECT_END) {
        return true;
    }
    return false;
}

bool isDurationVibration(Effect effectId) {
    return effectId == Effect::DURATION_ALARM_CALL ||
            effectId == Effect::DURATION_NOTIFICATION;
}

bool hasEffect(Effect effectId) {
    return sEffectMap.find(effectId) != sEffectMap.end() ||
            isDurationVibration(effectId);
}

Effect durationToEffectId(uint64_t duration) {
    if (duration < 17) {
        return Effect::DURATION_STRENGTH_LEVEL3;
    } else if (duration < 34) {
        return Effect::DURATION_STRENGTH_LEVEL6;
    } else if (duration < 51) {
        return Effect::DURATION_STRENGTH_LEVEL10;
    } else if (duration < 300) {
        return Effect::DURATION_NOTIFICATION;
    } else {
        return Effect::DURATION_ALARM_CALL;
    }
}

custom_effect* getEffect(Effect effectId, Style style) {
    if (sEffectMap.find(effectId) == sEffectMap.end()) {
        return nullptr;
    }
    auto& effect_style_set = sEffectMap[effectId];
    if (style == Style::CRISP) {
        return &effect_style_set.crisp;
    }
    return &effect_style_set.gentle;
}

effect_stream* loadFirmwareEffect(uint32_t effectId, Style style) {
    auto& effectDataMap = sFirmwareData[style];
    if (effectDataMap.find(effectId) != effectDataMap.end()) {
        return &effectDataMap[effectId];
    }

    std::string prefix = style == Style::CRISP ? kFirmwarePathPrefixDef : kFirmwarePathPrefixSoft;
    std::filesystem::path filePath = prefix + std::to_string(effectId) + ".bin";
    std::ifstream data(filePath, std::ios::in | std::ios::binary);
    if (!data.is_open()) {
        ALOGE("loadFirmwareEffect, failed to open %s for effect %d", filePath.string().c_str(), effectId);
        return nullptr;
    }

    uint32_t fileSize = std::filesystem::file_size(filePath);
    vector<int8_t> fifoData(fileSize);
    data.read(reinterpret_cast<char*>(fifoData.data()), fileSize);
    effectDataMap[effectId] = {
        .effect_id = effectId,
        .length = fileSize,
        .data = std::move(fifoData).data()
    };
    return &effectDataMap[effectId];
}
