#include "pin.h"
#include "key.h"

static u8 key_page = 0;
static u8 key_power = 0;

void KEY_Init(void)
{
}

//处理翻页按钮S2 
static void KEY_Page(void)
{
  if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)!=RESET) {
    key_page = 1;
  }
}

//处理 power 
static void KEY_Power(void)
{
  if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)!=RESET) {
    key_power = 1;
  }
}

void key_handle(struct key_trigger key)
{
  if ( key.trigger & KEY_PAGE_MASK) {
    KEY_Page();
  }
  
  if ( key.trigger & KEY_POWER_MASK) {
    KEY_Power();
  }
}

void get_key_status(struct key_status *state)
{
  state->page = key_page;
  state->power = key_power;
  
  key_page = 0;
  key_power = 0;
}