/*
 * Copyright (C) 2022-2024 The Nameless-AOSP Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "VibratorExt"

#include "include/VibratorExt.h"
#include <thread>

namespace vendor {
namespace nameless {
namespace hardware {
namespace vibratorExt {
namespace implementation {

#define test_bit(bit, array) ((array) [(bit) / 8] & (1 << ((bit) % 8)))

#define MIN_MAGNITUDE       0x1111
#define MAX_MAGNITUDE       0x7fff

#define INVALID_VALUE       -1
#define NAME_BUF_SIZE       32

#define EFFECT_TEXTURE_TICK 21

const char *kInputDir = "/dev/input/";

static const LevelRange kAlarmCallLevelRange = {
    .maxLevel = 5,
    .defaultLevel = 5
};
static const LevelRange kHapticLevelRange = {
    .maxLevel = 14,
    .defaultLevel = 14
};
static const LevelRange kNotificationLevelRange = {
    .maxLevel = 5,
    .defaultLevel = 5
};
static const LevelRange kUnknownLevelRange = {
    .maxLevel = 0,
    .defaultLevel = 0
};

uint32_t kAlarmCallLevel = kAlarmCallLevelRange.defaultLevel;
float kAlarmCallStrengthScale = (float) kAlarmCallLevel / kAlarmCallLevelRange.maxLevel;

uint32_t kHapticLevel = kHapticLevelRange.defaultLevel;
float kHapticStrengthScale = (float) kHapticLevel / kHapticLevelRange.maxLevel;

uint32_t kNotificationLevel = kNotificationLevelRange.defaultLevel;
float kNotificationStrengthScale = (float) kNotificationLevel / kNotificationLevelRange.maxLevel;

int kVibratorFd = INVALID_VALUE;
int16_t kCurrentAppId = INVALID_VALUE;

float kCurrentAmplitude = 1.0;
Style kCurrentStyle = Style::CRISP;

Effect kLastEffectId = Effect::DURATION_DEFAULT;
bool kWaitForInterval = false;

static int16_t getMagnitude(Effect effectId, float scale, bool fixedScale, bool hapticFeedback) {
    int16_t magnitude;
    if (kCurrentAmplitude == 1.0 || fixedScale) {
        if (hapticFeedback) {
            magnitude = (int16_t) ((kHapticLevel * (MAX_MAGNITUDE - MIN_MAGNITUDE)
                    / kHapticLevelRange.maxLevel + MIN_MAGNITUDE) * scale);
        } else if (effectId == Effect::DURATION_NOTIFICATION) {
            magnitude = (int16_t) ((kNotificationLevel * (MAX_MAGNITUDE - MIN_MAGNITUDE)
                    / kNotificationLevelRange.maxLevel + MIN_MAGNITUDE) * scale);
        } else {
            magnitude = (int16_t) ((kAlarmCallLevel * (MAX_MAGNITUDE - MIN_MAGNITUDE)
                    / kAlarmCallLevelRange.maxLevel + MIN_MAGNITUDE) * scale);
        }
    } else { 
        magnitude = (int16_t) (((uint8_t) (kCurrentAmplitude * 0xff) * (MAX_MAGNITUDE - MIN_MAGNITUDE)
                / 255 + MIN_MAGNITUDE) * scale);
    }
    if (DEBUG) {
        ALOGD("getMagnitude, effectId: %d, scale: %f, fixedScale: %d, hapticFeedback: %d, result: %hd",
                static_cast<int>(effectId), scale, fixedScale, hapticFeedback, magnitude);
    }
    return magnitude;
}

Return<void> VibratorExt::initVibrator() {
    int ret;

    DIR *dp = opendir(kInputDir);
    if (!dp) {
        ALOGE("initVibrator, open %s failed, errno = %d", kInputDir, errno);
        return Void();
    }

    uint8_t ffBitmask[FF_CNT / 8];
    memset(ffBitmask, 0, sizeof(ffBitmask));

    char devicename[PATH_MAX];
    dirent *dir;
    int fd;
    while ((dir = readdir(dp))) {
        if (dir->d_name[0] == '.' &&
                (dir->d_name[1] == '\0' ||
                (dir->d_name[1] == '.' && dir->d_name[2] == '\0'))) {
            continue;
        }

        snprintf(devicename, PATH_MAX, "%s%s", kInputDir, dir->d_name);
        fd = TEMP_FAILURE_RETRY(open(devicename, O_RDWR));
        if (fd < 0) {
            ALOGE("initVibrator, open %s failed, errno = %d", devicename, errno);
            continue;
        }

        char name[NAME_BUF_SIZE];
        ret = TEMP_FAILURE_RETRY(ioctl(fd, EVIOCGNAME(sizeof(name)), name));
        if (ret == INVALID_VALUE) {
            ALOGE("initVibrator, get input device name %s failed, errno = %d\n", devicename, errno);
            close(fd);
            continue;
        }

        if (strcmp(name, "qcom-hv-haptics")) {
            close(fd);
            continue;
        }

        if (DEBUG) {
            ALOGD("initVibrator, %s is detected at %s\n", name, devicename);
        }
        ret = TEMP_FAILURE_RETRY(ioctl(fd, EVIOCGBIT(EV_FF, sizeof(ffBitmask)), ffBitmask));
        if (ret == INVALID_VALUE) {
            ALOGE("initVibrator, ioctl failed, errno = %d", errno);
            close(fd);
            continue;
        }

        if (test_bit(FF_CONSTANT, ffBitmask) ||
                test_bit(FF_PERIODIC, ffBitmask)) {
            kVibratorFd = fd;
            break;
        }

        close(fd);
    }

    closedir(dp);

    return Void();
}

Return<int64_t> VibratorExt::vibratorOn(Effect effectId, uint64_t duration) {
    if (DEBUG) {
        ALOGD("vibratorOn, effectId: %d, duration: %lld", static_cast<int>(effectId), duration);
    }
    if (kVibratorFd < 0) {
        ALOGE("vibratorOn: input device is unavailable");
        vibratorOff();
        return INVALID_VALUE;
    }
    if (effectId == Effect::DURATION_DEFAULT) {
        if (DEBUG) {
            ALOGD("vibratorOn, effectId is DURATION_DEFAULT");
        }
        return vibratorOn(durationToEffectId(duration), duration);
    }
    if (!hasEffect(effectId)) {
        if (static_cast<int>(effectId) != EFFECT_TEXTURE_TICK) {
            ALOGE("vibratorOn: effectId %d not supported", static_cast<int>(effectId));
        }
        vibratorOff();
        return INVALID_VALUE;
    }
    if ((effectId == Effect::DURATION_ALARM_CALL || effectId == Effect::DURATION_NOTIFICATION)
            && duration <= 50) {
        if (DEBUG) {
            ALOGD("vibratorOn, use level 10");
        }
        return vibratorOn(Effect::DURATION_STRENGTH_LEVEL10, duration);
    }
    if (kWaitForInterval) {
        if (effectId == kLastEffectId) {
            if (DEBUG) {
                ALOGD("vibratorOn, waiting for interval, ignore same effect");
            }
            return INVALID_VALUE;
        }
        kWaitForInterval = false;
    }

    uint64_t playLengthMs = 0;
    int ret;
    custom_effect *customEffect = getEffect(effectId, kCurrentStyle);

    if (duration == 0 && !customEffect) {
        ALOGE("vibratorOn: duration is zero but effect %d not supported", static_cast<int>(effectId));
        vibratorOff();
        return INVALID_VALUE;
    }

    int cycles = customEffect ? customEffect->firmware_id.size() : 1;
    if (DEBUG) {
        ALOGD("vibratorOn, isEffect: %d, cycles size: %d", customEffect != nullptr, cycles);
    }

    for (int i = 0; i < cycles; i++) {
        if (kCurrentAppId != INVALID_VALUE) {
            TEMP_FAILURE_RETRY(ioctl(kVibratorFd, EVIOCRMFF, kCurrentAppId));
            kCurrentAppId = INVALID_VALUE;
        }

        int16_t magnitude = customEffect ? getMagnitude(
            effectId, /* effectId */
            customEffect->scale.size() != 0 ? customEffect->scale[i] : 1.0, /* scale */
            customEffect->fixed_scale, /* fixedScale */
            true /* hapticFeedback */
        ) : getMagnitude(
            effectId, /* effectId */
            1.0, /* scale */
            false, /* fixedScale */
            false /* hapticFeedback */
        );

        const uint32_t firmwareId = customEffect ? customEffect->firmware_id[i] : 0;
        const Style useStyle = !customEffect || customEffect->style == kStyleFollowUser ?
                kCurrentStyle : static_cast<Style>(customEffect->style);
        const effect_stream *stream = customEffect && firmwareId <= kFirmwareIdMax
                ? loadFirmwareEffect(firmwareId, useStyle) : nullptr;
        if (customEffect && firmwareId <= kFirmwareIdMax && !stream) {
            ALOGE("vibratorOn: isEffect but firmware %d not available", firmwareId);
            vibratorOff();
            return INVALID_VALUE;
        }
        const uint64_t length = customEffect && !stream ? firmwareId - kFirmwareIdMax : duration;
        if (DEBUG) {
            ALOGD("vibratorOn, firmwareId: %d, length=%lld, style=%d",
                    firmwareId, length, static_cast<int>(useStyle));
        }

        ff_effect effect;
        input_event play;
        memset(&effect, 0, sizeof(effect));
        if (stream) {
            effect.type = FF_PERIODIC;
            effect.u.periodic.waveform = FF_CUSTOM;
            effect.u.periodic.magnitude = magnitude;
            effect.u.periodic.custom_data = (int16_t *) stream;
            effect.u.periodic.custom_len = sizeof(*stream);
        } else {
            effect.type = FF_CONSTANT;
            effect.u.constant.level = magnitude;
            effect.replay.length = length;
        }

        effect.id = kCurrentAppId;
        effect.replay.delay = 0;

        ret = TEMP_FAILURE_RETRY(ioctl(kVibratorFd, EVIOCSFF, &effect));
        if (ret == INVALID_VALUE) {
            ALOGE("vibratorOn, ioctl EVIOCSFF failed, errno = %d", -errno);
            vibratorOff();
            return INVALID_VALUE;
        }

        kCurrentAppId = effect.id;
        if (stream) {
            playLengthMs += ((stream->length * 1000) / kPlayRateHz) + 1;
        } else {
            playLengthMs += length;
        }

        play.value = 1;
        play.type = EV_FF;
        play.code = kCurrentAppId;
        play.time.tv_sec = 0;
        play.time.tv_usec = 0;
        ret = TEMP_FAILURE_RETRY(write(kVibratorFd, (const void*) &play, sizeof(play)));
        if (ret == INVALID_VALUE) {
            ALOGE("vibratorOn, write failed, errno = %d\n", -errno);
            vibratorOff();
            return INVALID_VALUE;
        }

        if (i != cycles - 1 && customEffect && customEffect->sleep_time.size() != 0 && customEffect->sleep_time[i] > 0) {
            playLengthMs += customEffect->sleep_time[i];
            usleep(customEffect->sleep_time[i] * 1000);
        }
    }
    kCurrentAmplitude = 1.0;

    if (DEBUG) {
        ALOGD("vibratorOn, done, playLengthMs=%lld", playLengthMs);
    }

    if (customEffect && playLengthMs < customEffect->min_interval) {
        std::thread([=] {
            kWaitForInterval = true;
            kLastEffectId = effectId;
            usleep((customEffect->min_interval - playLengthMs) * 1000);
            kWaitForInterval = false;
        }).detach();
        return customEffect->min_interval;
    }
    return playLengthMs;
}

Return<void> VibratorExt::vibratorOff() {
    if (DEBUG) {
        ALOGD("vibratorOff, waitForInterval: %d", kWaitForInterval);
    }
    if (kWaitForInterval) {
        return Void();
    }
    if (kCurrentAppId != INVALID_VALUE) {
        TEMP_FAILURE_RETRY(ioctl(kVibratorFd, EVIOCRMFF, kCurrentAppId));
    }
    kCurrentAppId = INVALID_VALUE;
    kCurrentAmplitude = 1.0;
    return Void();
}

Return<void> VibratorExt::setAmplitude(float amplitude) {
    if (DEBUG) {
        ALOGD("setAmplitude, amplitude: %f", amplitude);
    }
    kCurrentAmplitude = amplitude;
    return Void();
}

Return<void> VibratorExt::setHapticStyle(Style style) {
    if (DEBUG) {
        ALOGD("setHapticStyle, style: %d", static_cast<int>(style));
    }
    kCurrentStyle = style;
    return Void();
}

Return<void> VibratorExt::getStrengthLevelRange(Type type, getStrengthLevelRange_cb callback) {
    switch (type) {
        case Type::ALARM_CALL:
            callback(kAlarmCallLevelRange);
            break;
        case Type::HAPTIC:
            callback(kHapticLevelRange);
            break;
        case Type::NOTIFICATION:
            callback(kNotificationLevelRange);
            break;
        default:
            ALOGE("getStrengthLevelRange, unknown vibration type: %d", static_cast<int>(type));
            callback(kUnknownLevelRange);
            break;
    }
    return Void();
}

Return<void> VibratorExt::setStrengthLevel(Type type, uint32_t level) {
    if (DEBUG) {
        ALOGD("setStrengthLevel, type: %d, level: %d", static_cast<int>(type), level);
    }
    switch (type) {
        case Type::ALARM_CALL:
            kAlarmCallLevel = level;
            kAlarmCallStrengthScale = (float) kAlarmCallLevel / kAlarmCallLevelRange.maxLevel;
            break;
        case Type::HAPTIC:
            kHapticLevel = level;
            kHapticStrengthScale = (float) kHapticLevel / kHapticLevelRange.maxLevel;
            break;
        case Type::NOTIFICATION:
            kNotificationLevel = level;
            kNotificationStrengthScale = (float) kNotificationLevel / kNotificationLevelRange.maxLevel;
            break;
        default:
            ALOGE("setStrengthLevel, unknown vibration type: %d", static_cast<int>(type));
            break;
    }
    return Void();
}

Return<bool> VibratorExt::isEffectSupported(Effect effectId) {
    return hasEffect(effectId);
}

Return<bool> VibratorExt::isHapticStyleSupported(Style style) {
    return style == Style::CRISP || style == Style::GENTLE;
}

}  // namespace implementation
}  // namespace vibratorExt
}  // namespace hardware
}  // namespace nameless
}  // namespace vendor
