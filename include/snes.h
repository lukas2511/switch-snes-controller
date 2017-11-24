#define SNES_CLOCK GPIO2
#define SNES_LATCH GPIO1
#define SNES_DATA  GPIO0

union SnesController {
  struct {
    unsigned int B : 1;
    unsigned int Y : 1;
    unsigned int select : 1;
    unsigned int start : 1;
    unsigned int dpad_up : 1;
    unsigned int dpad_down : 1;
    unsigned int dpad_left : 1;
    unsigned int dpad_right : 1;
    unsigned int A : 1;
    unsigned int X : 1;
    unsigned int L : 1;
    unsigned int R : 1;
    unsigned int : 4; // unused
  } data;
  uint8_t bytes[2];
};

void snes_init(void);
void snes_poll(fibre_t *fibre, uint8_t *bytes);
