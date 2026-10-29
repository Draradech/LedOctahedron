# hpwit ESP32-S3 full-frame test

Current configuration: **full-frame DMA, NO_WAIT, one persistent RGB array**.
Validated with Arduino ESP32 core **3.3.12**, eight outputs of 180 RGB pixels,
Wi-Fi AP and ArduinoOTA enabled. Only 60 LEDs on each of four outputs are
physically attached during this test.

## Library and build

The Git clone is installed at:
`/home/draradech/Arduino/libraries/I2SClocklessLedDriver`

Upstream: https://github.com/hpwit/I2SClocklessLedDriver
Base commit: `a736f5ecb1d71056521f8054295a8326908550d4`

The clone has a local full-frame repair in `src/I2SClocklessLedDriver.h`.
Its `docs/developer/full-frame-esp32s3.md` explains the changes and validation.
The unmodified upstream commit does not compile with `FULL_DMA_BUFFER` on S3.
No FastLED dependency or custom output driver is used by this sketch.

`build_opt.h` contains:

```text
-O3
-DFULL_DMA_BUFFER
```

The ESP32 Arduino core applies these options to the sketch and library, so the
feature macro is consistent across translation units. Force a clean rebuild
when changing these options. Open `HPWITTest.ino` in Arduino IDE using ESP32-S3
Dev Module, Hardware CDC and JTAG, USB CDC On Boot enabled, PSRAM disabled.
Do not replace the patched Git clone with the unmodified upstream ZIP.

## Runtime

- GPIO outputs: 8, 9, 10, 11, 13, 14, 15, 16; GRB wire order, brightness 255.
- Same moving-white test as the FastLED baseline: first 60 pixels of outputs
  0–3 animate; the remaining configured pixels stay black.
- `showPixelsFirstTranspose(NO_WAIT)` waits for the previous DMA frame if
  needed, encodes the complete waveform, starts DMA, and returns. The RGB
  source array can be edited immediately afterward; DMA owns a separate
  staged waveform. There is no second RGB source buffer.
- The 15-second startup OTA recovery window runs before LED initialization.
  OTA drains pending LED output before flash operations.
- Serial diagnostics are nonblocking and may drop reports without a reader.
  Avoid terminal control-line transitions that reset USB-Serial/JTAG.

## Checks and results

Startup verifies the 182-descriptor, 28112-byte chain with one EOF at the reset
low tail and zero prefix/tail data. It exercises repeated WAIT submissions,
immediate NO_WAIT resubmission, source mutation without changing staged data,
loop start/stop and return to single-shot, and sync-semaphore reuse. Failure
leaves animation disabled with OTA available.

The first full-frame test sustained 120 FPS with staging/submission around
0.7 ms and visually smooth output without blips. A finite synthetic 5 ms CPU
rendering load sustained 120.004 FPS across 19 complete steady windows, with
maximum staging/submission 770 us and maximum frame interval 8428 us. Every
frame in those windows edited the RGB array while the previous DMA transfer
was active.

That load is **disabled** in the normal build. To reproduce, enable
`HPWIT_RENDER_STRESS_TEST` in `led.cpp`; it applies the load from 5 to 25 seconds
after the animation starts, then automatically returns to normal. Diagnostics
include render time and the number of source edits overlapping DMA output.

The committed streaming sketch was separately compiled and smoke-tested
against the patched library on core 3.3.12: 120 FPS and about 5.7 ms for blocking
show, with steady heap. Earlier streaming tests also verified output with the
serial reader closed for 20 seconds. Actual eight-lane, full-length chains,
other chip targets and RGBW/RGBCCT output remain outside this hardware test.

Final normal-build validation on core 3.3.12: 75 steady windows measured
120.0048 FPS, maximum staging/submission 738 us, maximum reported frame
interval 8412 us, and free heap 146016–146040 B. All startup checks passed;
the synthetic load was disabled. This image is left running and the serial
reader has been closed without toggling the reset signals.
