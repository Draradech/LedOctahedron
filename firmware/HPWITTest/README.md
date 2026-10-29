# hpwit ESP32-S3 comparison test

This sketch tests the unmodified hpwit `I2SClocklessLedDriver` at commit
`a736f5ecb1d71056521f8054295a8326908550d4` (main at the time of testing).
Repository: https://github.com/hpwit/I2SClocklessLedDriver

- Eight outputs: GPIO 8, 9, 10, 11, 13, 14, 15, 16.
- 180 RGB pixels on each output, GRB wire order, brightness 255.
- Same test animation as the FastLED baseline: one moving white pixel in the
  first 60 pixels of outputs 0–3; all other pixels stay black.
- Target 120 FPS; default streaming DMA, using blocking `showPixels(WAIT)`.
- Wi-Fi AP and OTA remain enabled. Startup offers a 15-second OTA recovery
  window before the driver initializes. OTA waits for outstanding DMA.
- Native USB diagnostics are nonblocking and may drop lines if not consumed.
- `build_opt.h` requests `-O3`. No FastLED dependency or local library patches
  are used by this test.

The stock `FULL_DMA_BUFFER` configuration does not compile on ESP32-S3:
its full-buffer code uses ESP32-only `descriptor` fields, and references an
undefined `W` variable. Those errors are recorded in `work/hpwit-full-compile.log`.
This test does not enable that mode or patch the upstream library.

Install the pinned library via a ZIP or supply its directory with Arduino
CLI's `--library` option. For Arduino IDE use ESP32-S3 Dev Module, Hardware
CDC and JTAG, USB CDC On Boot enabled, PSRAM disabled, Arduino ESP32 3.3.11.
Open `HPWITTest.ino`. The FastLED baseline remains in `../LEDOctahedron`.

## Initial hardware result

The stock streaming mode compiled and was flashed via OTA. With Wi-Fi AP and
OTA enabled, it maintained 120 updates/s; the measured blocking show call was
about 5.7 ms including transmission. The user reported smooth output with no
pixel blips. All eight 180-pixel lanes were configured; only the first 60 LEDs
on four lanes were physically connected. Free heap stabilized near 211,576 B.
These observations do not validate the unconnected pixels or the broken
full-frame mode.

### Reproduce the build

Download and extract the pinned source archive:
https://github.com/hpwit/I2SClocklessLedDriver/archive/a736f5ecb1d71056521f8054295a8326908550d4.zip

Then compile with Arduino CLI (replace the library directory with the extracted
folder):

```sh
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc \
  --library /path/to/I2SClocklessLedDriver-a736f5ecb1d71056521f8054295a8326908550d4 \
  firmware/HPWITTest
```

For Arduino IDE, install that library ZIP using Sketch > Include Library > Add
.ZIP Library, then open this sketch. No installed-library sources were changed
for the hpwit test. Do not enable `FULL_DMA_BUFFER` on this commit.

With exclusive serial access, a 64-window steady-state sample measured
120.004 updates/s, a maximum reported inter-frame interval of 8,391 us, and a
maximum show duration of 5,723 us. Free heap was constant at 211,572 B. A first
reader-close test deliberately changed DTR and reset the USB-Serial/JTAG board;
that is not a valid test of whether output continues without a serial reader.
Use a terminal configuration that does not toggle the board's reset lines.

The corrected reader-close test cleared DTR/RTS together and disabled hangup
control-line changes. With the reader closed for 20 seconds, the frame counter
advanced from 4,204 to 8,048 over 32.03331 seconds including the reconnect sample:
120.0001 updates/s, with no reset. The subsequent measured rate was 120.004
updates/s, max frame interval 8,390 us, max show duration 5,718 us, and constant
free heap of 211,568 B. The serial port was closed after the test without
changing reset lines. The hpwit streaming image is left running on the board.
