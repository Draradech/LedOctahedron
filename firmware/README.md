# Firmware

Open `LEDOctahedron/LEDOctahedron.ino` in Arduino IDE, board **ESP32S3 Dev Module** (tested with ESP32 core **3.3.12**). Enable **USB CDC On Boot** for `REPORT_SERIAL` over USB.

Requires the [I2SClocklessLedDriver fork](https://github.com/Draradech/I2SClocklessLedDriver/tree/32s3_repair_fullframe), branch `32s3_repair_fullframe`, with the ESP32-S3 full-frame repair.

`build_opt.h` enables `-O3` and `FULL_DMA_BUFFER`. Reporting is selected in `stdinc.h`: off, serial, or UDP (default, broadcast port 7777).
