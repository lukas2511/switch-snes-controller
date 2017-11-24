#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include <librfn/fibre.h>
#include <librfn/time.h>

#include "include/controllers.h"
#include "include/usb.h"
#include "include/snes.h"
#include "include/bitset.h"

union SwitchController switch_controller;
union SnesController snes_controller;

static int usb_fibre(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  while(1) {
    usb_poll();
    PT_YIELD();
  }
  PT_END();
}
static fibre_t usb_task = FIBRE_VAR_INIT(usb_fibre);

static bool first_run = 1;
static bool fake_analog = 0;

static int main_loop(fibre_t *fibre) {
  PT_BEGIN_FIBRE(fibre);
  static uint32_t t;
  static uint32_t i;

  for(i=0; i<sizeof(switch_controller.bytes); i++) {
    switch_controller.bytes[i] = 0;
  }

  while(1) {
    // 1ms timeout
    t = time_now() + 1000;
    PT_WAIT_UNTIL(fibre_timeout(t));
    gpio_toggle(GPIOC, GPIO13);

    if(usb_running) {
      // TODO: read snes controller
      gpio_set(GPIOA, SNES_CLOCK);
      gpio_clear(GPIOA, SNES_LATCH);

      t = t+1000;
      PT_WAIT_UNTIL(fibre_timeout(t));

      gpio_set(GPIOA, SNES_LATCH);
      t = t+1000;
      PT_WAIT_UNTIL(fibre_timeout(t));
      gpio_clear(GPIOA, SNES_LATCH);
      t = t+1000;
      PT_WAIT_UNTIL(fibre_timeout(t));


      for(i=0; i<16; i++) {
        gpio_toggle(GPIOA, SNES_CLOCK);
        t = t+1000;
        PT_WAIT_UNTIL(fibre_timeout(t));


        if(gpio_get(GPIOA, SNES_DATA)) {
          _CLRBIT(snes_controller.bytes[i/8], i % 8);
        } else {
          _SETBIT(snes_controller.bytes[i/8], i % 8);
        }

        gpio_toggle(GPIOA, SNES_CLOCK);
        t = t+1000;
        PT_WAIT_UNTIL(fibre_timeout(t));
      }

      if(first_run && snes_controller.data.A) {
        fake_analog = 1;
      }
      first_run = 0;

      switch_controller.data.A = snes_controller.data.A;
      switch_controller.data.B = snes_controller.data.B;
      switch_controller.data.X = snes_controller.data.X;
      switch_controller.data.Y = snes_controller.data.Y;
      switch_controller.data.L = snes_controller.data.L;
      switch_controller.data.R = snes_controller.data.R;
      switch_controller.data.minus = snes_controller.data.select;
      switch_controller.data.plus = snes_controller.data.start;

      switch_controller.data.RX = 127;
      switch_controller.data.RY = 127;

      if(fake_analog) {
        switch_controller.data.LX = 127 - (127 * snes_controller.data.dpad_left) + (128 * snes_controller.data.dpad_right);
        switch_controller.data.LY = 127 - (127 * snes_controller.data.dpad_up) + (128 * snes_controller.data.dpad_down);
        switch_controller.data.dpad_up = 0;
        switch_controller.data.dpad_down = 0;
        switch_controller.data.dpad_left = 0;
        switch_controller.data.dpad_right = 0;
      } else {
        switch_controller.data.LX = 127;
        switch_controller.data.LY = 127;
        switch_controller.data.dpad_up = snes_controller.data.dpad_up;
        switch_controller.data.dpad_down = snes_controller.data.dpad_down;
        switch_controller.data.dpad_left = snes_controller.data.dpad_left;
        switch_controller.data.dpad_right = snes_controller.data.dpad_right;
      }

      usb_write(switch_controller.bytes, 8);
    }
  }
  PT_END();
}
static fibre_t main_loop_task = FIBRE_VAR_INIT(main_loop);

int main(void) {
  rcc_clock_setup_in_hsi_out_48mhz();
  rcc_periph_clock_enable(RCC_GPIOC);
  gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);

  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, SNES_CLOCK);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, SNES_LATCH);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, SNES_DATA);

  time_init();

  init_usb();

  fibre_run(&usb_task);
  fibre_run(&main_loop_task);
  fibre_scheduler_main_loop();
}
