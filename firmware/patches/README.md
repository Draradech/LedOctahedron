# FastLED LCD_CAM configuration and local patches

Tested with the installed FastLED 3.10.8 and Arduino ESP32 3.3.11 on ESP32-S3.
The application uses the typed Channels API: `FLEX_IO` instance 0 resolves to
`LCD_CLOCKLESS`. All eight lanes transmit 180 pixels; the current test animation
only changes the first 60 pixels of lanes 0–3.

The sketch's `build_opt.h` requests `-O3` using the Arduino ESP32 core's supported
per-sketch compiler options mechanism. Use USB Mode **Hardware CDC and JTAG**
and **USB CDC On Boot: Enabled** for diagnostics on the native USB connector.
Diagnostics use nonblocking, best-effort writes so a missing serial reader
cannot stop the animation. The frame schedule targets 120 Hz.

Two local FastLED patches are already applied to
`/home/draradech/Arduino/libraries/FastLED`:

- `fastled-lcd-cache-sync.patch`: only write back cache for external RAM.
  Internal DMA RAM does not require it and ESP-IDF rejects that address range.
- `fastled-lcd-eight-lane.patch`: use the existing eight-lane wave3 encoder for
  up to eight outputs, then expand to the original 16-bit DMA word layout.
  The unused upper lanes retain the same zero-data waveform as before.

Reinstalling or updating FastLED may overwrite these changes. To reapply to an
unpatched, compatible library, run from its root:

```sh
patch -p1 < /path/to/LedOctahedron/firmware/patches/fastled-lcd-cache-sync.patch
patch -p1 < /path/to/LedOctahedron/firmware/patches/fastled-lcd-eight-lane.patch
```

The sketch sets the driver's existing `setChunkInputBytesForTest(NUM_LEDS * 3)`
knob to encode a full lane before starting a single DMA transaction. Small
streaming chunks produced visible pixel blips on the hardware and are disabled.
The eight-lane encoder optimization remains enabled. The chunk-size setter is
an internal driver API; check compatibility when changing FastLED versions.
No custom LED driver is added.

`../tests/lcd-wave3-equivalence.cpp` compares the eight-lane packing against
FastLED's original 16-lane encoder for 10,000 deterministic random vectors
with 1–8 active lanes. It can be built on the host with the FastLED headers and
its `src/fl/channels/wave3.cpp.hpp` and
`src/fl/channels/detail/wave3.cpp.hpp` sources, defining `FASTLED_STUB_IMPL` and
using function/data sections plus linker garbage collection.

Temporary phase profiling in FastLED itself has been removed. Only the two
patches above remain in the installed library. OTA drains pending LED DMA
before flash/cache operations.
