# sWiitch controller

Code to use a SNES controller on the Nintendo Switch

Build using libopencm3 for the stm32f103 series.

[![demo video](https://img.youtube.com/vi/fOCP0zd441E/0.jpg)](https://www.youtube.com/watch?v=fOCP0zd441E)

## Hardware

I'm using a stm32f108c8t6 on a so called "blue pill" board, which basically is just the bare minimum
to get the chip running.
I had to solder a 1.8kOhm resistor on the board between A12 and 3.3V to get USB working (see [stm32duino wiki](http://wiki.stm32duino.com/index.php?title=Blue_Pill#Hardware_installation)).

The SNES controller is connected to GND, 3.3V, A0 (DATA_OUT), A1 (LATCH) and A2 (CLOCK).

## Modes

By default the button mapping is 1:1 from the SNES labels to Switch buttons, including the DPAD.
For some games this setup is not working since the dpad can't be used for movement.

When pressing A when plugging then controller in the DPAD becomes a fake analog stick.
Pressing L results the L button to be mapped to ZL (e.g. ground pound in Mario Odyssey), same for R/ZR.

When pressing Select and Start at the same time it simulates pressing the home button.

## Bootloader

I'm using Daniel Thompson's modified version of the example libopencm3 bootloader, which can be found in the [i2c-star repository](https://github.com/daniel-thompson/i2c-star/tree/master/src/bootloader).

I changed VID/PID to match the controller so dfu-util can automatically reprogram the device without a manual reset:

```diff
diff --git a/src/bootloader/usbdfu.c b/src/bootloader/usbdfu.c
index ba6ce07..97be28a 100644
--- a/src/bootloader/usbdfu.c
+++ b/src/bootloader/usbdfu.c
@@ -54,8 +54,8 @@ const struct usb_device_descriptor dev = {
        .bDeviceSubClass = 0,
        .bDeviceProtocol = 0,
        .bMaxPacketSize0 = 64,
-       .idVendor = 0x0483,
-       .idProduct = 0xDF11,
+       .idVendor = 0x0f0d,
+       .idProduct = 0x00c1,
        .bcdDevice = 0x0200,
        .iManufacturer = 1,
        .iProduct = 2,
```
