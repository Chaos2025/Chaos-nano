#ifndef _KEY_H
#define _KEY_H

struct key_trigger{
  u8 trigger;
};

struct key_status{
  u8 power;
  u8 page;
};

#define KEY_PAGE_MASK   (1 << 0)
#define KEY_POWER_MASK   (1 << 1)

void KEY_Init(void);
void key_handle(struct key_trigger key);
void get_key_status(struct key_status *state);

#endif