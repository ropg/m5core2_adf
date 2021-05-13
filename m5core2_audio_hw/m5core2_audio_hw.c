/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2020 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <string.h>
#include "esp_log.h"
#include "m5core2_axp192.h"
#include "board.h"

#include "m5core2_audio_hw.h"

static const char *TAG = "M5CORE2_ADF";

static bool codec_init_flag;

audio_hal_func_t AUDIO_M5CORE2_AUDIO_HW_DEFAULT_HANDLE = {
    .audio_codec_initialize = m5core2_audio_hw_init,
    .audio_codec_deinitialize = m5core2_audio_hw_deinit,
    .audio_codec_ctrl = m5core2_audio_hw_ctrl_state,
    .audio_codec_config_iface = m5core2_audio_hw_config_i2s,
    .audio_codec_set_mute = m5core2_audio_hw_set_voice_mute,
    .audio_codec_set_volume = m5core2_audio_hw_set_voice_volume,
    .audio_codec_get_volume = m5core2_audio_hw_get_voice_volume,
};

bool m5core2_audio_hw_initialized()
{
    return codec_init_flag;
}

esp_err_t m5core2_audio_hw_init(audio_hal_codec_config_t *cfg)
{
    esp_err_t ret = ESP_OK;
    ret |= m5core2_init();
    ret |= m5core2_audio_hw_set_voice_mute(false);
    return ret;
}

esp_err_t m5core2_audio_hw_deinit(void)
{
	esp_err_t ret;
 	ret = m5core2_audio_hw_set_voice_mute(true);
    return ret;
}

esp_err_t m5core2_audio_hw_ctrl_state(audio_hal_codec_mode_t mode, audio_hal_ctrl_t ctrl_state)
{
    return ESP_OK;
}

esp_err_t m5core2_audio_hw_config_i2s(audio_hal_codec_mode_t mode, audio_hal_codec_i2s_iface_t *iface)
{
    return ESP_OK;
}

esp_err_t m5core2_audio_hw_set_voice_mute(bool mute)
{
	ESP_LOGI(TAG, "Amplifier turned %s", mute ? "off" : "on");
	return m5core2_speaker(!mute);
}

esp_err_t m5core2_audio_hw_set_voice_volume(int volume)
{
    return ESP_OK;
}

esp_err_t m5core2_audio_hw_get_voice_volume(int *volume)
{
    return ESP_OK;
}
