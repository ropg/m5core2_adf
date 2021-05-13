# M5Core2 ESP-ADF audio component

*"Finally, someone got the M5Core2 to work as an audio board for ESP-ADF !"*

&nbsp;


### Playing audio: just works


```bash
# copy example from the ADF directory to current directory
cp -r [...]/esp-adf/examples/get-started/play_mp3 .

# install this component and dependencies
cd play_mp3/components
rm -rf my_board
git clone https://github.com/ropg/m5core2_adf
git clone https://github.com/ropg/m5core2_axp192
git clone https://github.com/ropg/i2c_manager
git clone https://github.com/tuupola/axp192

# Add some values to the Kconfig default configuration
cd ..
cat << EOF >> sdkconfig.defaults
CONFIG_I2C_MANAGER_0_ENABLED=y
CONFIG_I2C_MANAGER_0_SDA=21
CONFIG_I2C_MANAGER_0_SCL=22
CONFIG_I2C_MANAGER_0_FREQ_HZ=1000000
CONFIG_I2C_MANAGER_0_TIMEOUT=20
CONFIG_I2C_MANAGER_0_LOCK_TIMEOUT=50
CONFIG_AUDIO_BOARD_CUSTOM=y
EOF

# Build, flash and run
idf.py flash monitor
```

As you can see, examples that just play sound work out of the box. No need to change any source code, you're good to go. (The `CONFIG_` defines above make sure 'Custom Audio Board' is selected in ESP-ADF's 'Audio HAL' section of the config and that the I2C port is set up to talk to the Power Management IC.)

### Microphone: different stream configuration

To open an input stream, you must replace the `I2S_STREAM_CFG_DEFAULT()` the examples generally use to open the stream with our `I2S_STREAM_CFG_M5CORE2_MICROPHONE()`.

So if you do the same as above with `esp-adf/examples/recorder/pipeline_wav_sdcard` and do this replacement in `app_main()` before compiling, recording will also just work. (You'll need to insert a VFAT-formatted SD-card.)

### Full duplex: no

To the best of my knowledge, **the M5Core2 cannot do full-duplex audio.** (See below for much more explanation.) Any code that keeps two I2S streams open at the same time – whether it actively uses them in a full-duplex fashion or not – needs to be modified to have only one stream open at any time.

&nbsp;


## Working with ESP-ADF

### Voice Recognition

One of the more exciting parts of ESP-ADF is the speaker independent voice recognition. The example for that needs to be modified because it plays sounds, (like a dingdong when it recognizes the 'wake word') and to do this it keeps two streams open simultaneously. As mentioned above, the M5Core2 does no have full-duplex audio, so this needs to be modified to switch back and forth. For the purposes of this demo, we'll just strip this code out because we can see what's going on on the serial monitor. To do this, do the same things as above to the code at `esp-adf/examples/speech_recognition/asr`.

First find these lines:

```c
ESP_LOGI(TAG, "[ 2.1 ] Create i2s stream to read audio data from codec chip");
i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
```
and replace `I2S_STREAM_CFG_DEFAULT()` with `I2S_STREAM_CFG_M5CORE2_MICROPHONE()`. When that's done, find and remove (or comment out) the following lines:

```c
audio_board_sdcard_init(esp_periph_set_init(&periph_cfg), SD_MODE_1_LINE);
```
```c
player = setup_player();
```
```c
esp_audio_sync_play(player, "file://sdcard/haode.mp3", 0);
```
```c
esp_audio_sync_play(player, "file://sdcard/dingdong.mp3", 0);
```

Now when you run it with `idf.py flash monitor`, you will see it start up and you'll have to say 'Hi Lexin' near the device until it outputs `example_asr_keywords: wake up` on the serial monitor and then – in your best Mandarin – say one of the phrases from the [README](https://github.com/espressif/esp-adf/tree/master/examples/speech_recognition/asr) for this example. Note that I do not actually speak Manadrin, so I can't judge if the recognition is any good. But I did get it to recognize the phrase I was actually trying to say a few times. Your mileage may vary.

```txt
[...]
I (230848) example_asr_keywords: [ 5 ] Start audio_pipeline
I (239609) example_asr_keywords: wake up
I (247156) example_asr_keywords: stop multinet
I (406821) example_asr_keywords: wake up
I (410192) example_asr_keywords: increase in temperature
```

#### Notes:

   * ESP-ADF, our removals and this code by itself seem to produce compiler warnings, but they do seem to impact functionality.
   * If you replace `esp_log_level_set("*", ESP_LOG_WARN);` with `esp_log_level_set("*", ESP_LOG_INFO);`, you'll get a better idea of what is happening.
   * There are a few supplied defaults, but introduction of new wake words in the `wakenet` involves Espressif taking money and time. It would be nice if we could train this ourselves.
   * I do not know yet what it takes to make the `multinet`, the part that recognizes the words after the wakeword, recognize different words. More research is needed.


&nbsp;


### Streaming

The ESP-ADF code can stream to and from the network, some examples let you configure an SSID and a password for that. I've tried some of the streaming examples, and I have been unimpressed because of constant pauses. I presume increasing the ring buffer sizes between some components of the audio pipeline would help, but I've seen example code for some Arduino streaming libraries perform much better out of the box.

&nbsp;


### Other Peripherals

The esp-adf examples sometimes talk to buttons on the Espressif factory audio boards, and there's theoretical hooks for putting inputs like 'play' and 'stop' buttons on touch screens. I'm not sure it's worthwhile implementing any of this as it would limit any other UI you are working on to have to do it 'the ESP-ADF way'. I recommend you just use the audio functionality and do not see ESP-ADF as some more general interfacing framework.

&nbsp;

## Creating `M5Core2_adf`

There was a bit of work involved in getting this all to work. Part of it is me finding my way, but there are also some issues with how audio works on the M5Core2 and with how ESP-ADF has implemented custom audio boards. We'll cover some difficult bits. If you're just using this component you should need anything from below here, but I'm writing this just so it's all documented somewhere.

#### GPIO_0 used for MCLK output

I modified the custom drivers that came with the `play_mp3` example to what I thought should work for the M5Core2, made sure the speaker amplifier was on and then could not figure out why I couldn't get it to output anything for the longest time. What I had overlooked was the little sentence `I2S0, MCLK output by GPIO0` in the debug output. There is indeed an I2S clock signal on that pin in the M5Core2, namely the "Word Select" (WS), a.k.a. Left-Right Clock" (LRCLK), a.k.a. Frame Sync (FS). As you can tell, the same signals can have many names in I2S-land, so I missed that MCLK is actually a different, very fast signal (much faster than the bit clock of I2S) sometimes used by delta-sigma modulators and digital filters. We don't need that signal on M5Core2, and having it be put where the speaker amplifier expects its left-right clock messes things up.

I was surprised to find this hardcoded in the ESP-ADF's `components/audio_stream/i2s_stream.c`:

```c
i2s_mclk_gpio_select(i2s->config.i2s_port, GPIO_NUM_0);
```

This code has three levels of abstraction for lots of things, and then simply hardcodes that I am supposed to want this on GPIO 0?

Fortunately, the `i2s_mclk_gpio_select` function is in the custom audio board driver code, so I could just replace it with:

```
esp_err_t i2s_mclk_gpio_select(i2s_port_t i2s_num, gpio_num_t gpio_num) {
    ESP_LOGI(TAG, "I2S port %d, no MCLK output on GPIO%d for M5Core2", i2s_num, gpio_num);
    return ESP_OK;
}
```
([Issue](https://github.com/espressif/esp-adf/issues/618) filed.)

#### Half-Duplex

The M5Core2 audio subsystem consists of an I2S microphone component and an I2S amplifier IC with a speaker. Unformtunately they share the clock wires while needing incompatble I2S settings. The microphone operates in PDM or Pulse Density Modulation, the amplifier chip uses regular PCM. I have experimented trying to use the second I2S peripheral on the same clock wires, but have not managed to get this to work. Maybe some ESP32 low-level IO-register guru can get it to work by using one of the perpiherals as a slave of the other's clock, but until then I think we have to accept that there can only be one direction configured at a time. (And at least for the microphone it needs to be using the first built-in I2S port `I2S_NUM_0`, because `I2S_NUM_1` does not support PDM.)

#### Low-Level, DC-Bias, power-on excursion

The microphone level is really low, to be usable the signal needs to be digitally multiplied. The microphone hardware, about 120 ms after the stream being opened, sends the level down about to about twice the maximal speech volume and then over the course of a second returns to swinging around zero again, even though a noticable negative DC-bias remains. In my `I2S_STREAM_CFG_M5CORE2_MICROPHONE` settings I have turned on ESP-ADF's native ALC (Automatic Level Control) to fix things. This makes things louder, including significant noise. I have expermented with not feeding the first second to ALC, but couldn't get enough improvement out of that to justify a mike-only special version of `i2s_stream`. Signal is a bit muffled because of the placement of the microphone on the extension board and the sound having to make its way to it. There is an equalizer audio component that can conceivably be used to fix the audio a bit more.

> *You might want to have a look at this [video](https://www.youtube.com/watch?v=CwIWpBqa-nM) where 'atomic14' goes over the M5Core2 audio input in detail.*

#### SD-Card SPI pins

There is no way to say this nicely: **they hardcoded the SPI pins to the SD-Card!** The pins to the SD-card slot on the officially santioned Espressif audio boards are right there in the code with no way to override.

`esp-adf/components/esp_peripherals/lib/sdcard/sdcard.c` says:

```c
#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13
```

These are the same people that then offer a function `get_sdcard_intr_gpio` in the driver code to figure out the 'card present' GPIO pin. I have no clue what they were thinking. The easiest way to solve this would have been to add an `#ifndef PIN_NUM_MISO` around these pins, but that would mean modifying the ESP-ADF code. Which in turn means you can't just `git checkout` different versions, and it would also mean having an M5Core2 driver that doesn't 'just work', which was a major objective here.

So I ended up duplicating the mount process for the SD-card in hacked versions of `sdcard.c` and `periph_sdcard.c` in the `sdcard_hack` directory of the `m5core2_adf` component. It all works, but a bit silly it is. ([Issue](https://github.com/espressif/esp-adf/issues/617) filed.)


&nbsp;

&nbsp;

#### Contribute

> I you found the information here useful, please consider starring this repository on GitHub, so others can find it more easily.

> Please feel free to share any useful information on using ESP-ADF with the M5Core2 device on this repository's [wiki](https://github.com/ropg/m5core2_adf/wiki).