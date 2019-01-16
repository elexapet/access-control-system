
#include "panel.h"

static void panel_timer_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (int32_t) pvTimerGetTimerID(pxTimer);

  // Lock state
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_beep_port, acc_panel[id].acc_panel_beep_pin, LOG_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_gled_port, acc_panel[id].acc_panel_gled_pin, LOG_HIGH);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].relay_port, acc_panel[id].relay_pin, LOG_HIGH);
}

void panel_init(uint8_t id)
{
  //Create stream buffer
  acc_panel[id].card_stream = xStreamBufferCreate(CONSUMER_BUFF_SIZE, CONSUMER_BUFF_SIZE);
  configASSERT(acc_panel[id].card_stream);

  //Init interface to reader
  weigand_init(acc_panel[id].card_stream, acc_panel[id].acc_panel_port, acc_panel[id].acc_panel_d0_pin, acc_panel[id].acc_panel_d1_pin);

  //Create timer
  if (acc_panel[id].timer == NULL)
  {
    acc_panel[id].timer = xTimerCreate("PNL_TMR", (acc_panel[id].open_time_sec / portTICK_PERIOD_MS), pdFALSE, (void *)(uint32_t) id, panel_timer_callback);
  }
  configASSERT(acc_panel[id].timer);

  //Setup LED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, acc_panel[id].acc_panel_gled_port, acc_panel[id].acc_panel_gled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[acc_panel[id].acc_panel_gled_port][acc_panel[id].acc_panel_gled_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_gled_port, acc_panel[id].acc_panel_gled_pin, LOG_HIGH);

  //Setup BEEPER
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, acc_panel[id].acc_panel_beep_port, acc_panel[id].acc_panel_beep_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[acc_panel[id].acc_panel_beep_port][acc_panel[id].acc_panel_beep_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_beep_port, acc_panel[id].acc_panel_beep_pin, LOG_HIGH);

  Chip_GPIO_SetPinDIROutput(LPC_GPIO, acc_panel[id].relay_port, acc_panel[id].relay_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[acc_panel[id].relay_port][acc_panel[id].relay_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].relay_port, acc_panel[id].relay_pin, LOG_HIGH);
}

void panel_deinit(uint8_t id)
{
  if (acc_panel[id].card_stream != NULL)
  {
    vStreamBufferDelete(acc_panel[id].card_stream);
    acc_panel[id].card_stream = NULL;
  }

  if (xTimerDelete(acc_panel[id].timer, 0) != pdFAIL)
  {
    acc_panel[id].timer = NULL;
  }

  weigand_disable(acc_panel[id].acc_panel_port, acc_panel[id].acc_panel_d0_pin, acc_panel[id].acc_panel_d1_pin);
}

void panel_unlock(uint8_t id)
{
  // Unlock state
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_beep_port, acc_panel[id].acc_panel_beep_pin, LOG_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_gled_port, acc_panel[id].acc_panel_gled_pin, LOG_LOW);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].relay_port, acc_panel[id].relay_pin, LOG_LOW);
  //Start timer
  xTimerStart(acc_panel[id].timer, 0);
}

