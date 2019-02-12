
#include "panel.h"

static StreamBufferHandle_t _reader_buffer;

panel_conf_t acc_panel[DOOR_ACC_PANEL_MAX_COUNT] =
{
  {
    .acc_panel_on = DOOR_0_ACC_PANEL_ON,
    .acc_panel_port = DOOR_0_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_0_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_0_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_0_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_0_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_0_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_0_ACC_PANEL_GLED_PIN,
    .relay_port = DOOR_0_RELAY_PORT,
    .relay_pin = DOOR_0_RELAY_PIN,
    .open_time_sec = DOOR_0_OPEN_TIME_MS,
    .timer = NULL,
  },
  {
    .acc_panel_on = DOOR_1_ACC_PANEL_ON,
    .acc_panel_port = DOOR_1_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_1_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_1_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_1_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_1_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_1_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_1_ACC_PANEL_GLED_PIN,
    .relay_port = DOOR_1_RELAY_PORT,
    .relay_pin = DOOR_1_RELAY_PIN,
    .open_time_sec = DOOR_1_OPEN_TIME_MS,
    .timer = NULL,
  },
  {
    .acc_panel_on = DOOR_2_ACC_PANEL_ON,
    .acc_panel_port = DOOR_2_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_2_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_2_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_2_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_2_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_2_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_2_ACC_PANEL_GLED_PIN,
    .relay_port = DOOR_2_RELAY_PORT,
    .relay_pin = DOOR_2_RELAY_PIN,
    .open_time_sec = DOOR_2_OPEN_TIME_MS,
    .timer = NULL,
  }
};

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
  //Create stream buffer to receive id from all card readers
  if (_reader_buffer == NULL)
  {
    _reader_buffer = xStreamBufferCreate(WEIGAND_DEVICE_LIMIT * WEIGAND26_BUFF_ITEM_SIZE, WEIGAND26_BUFF_ITEM_SIZE);
  }
  configASSERT(_reader_buffer);

  //Init interface to reader
  weigand_init(_reader_buffer, id, acc_panel[id].acc_panel_port, acc_panel[id].acc_panel_d0_pin, acc_panel[id].acc_panel_d1_pin);

  //Create timer only once
  if (acc_panel[id].timer == NULL)
  {
    acc_panel[id].timer = xTimerCreate("PNL", (acc_panel[id].open_time_sec / portTICK_PERIOD_MS), pdFALSE, (void *)(uint32_t) id, panel_timer_callback);
  }
  configASSERT(acc_panel[id].timer);

  //Setup LED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, acc_panel[id].acc_panel_gled_port, acc_panel[id].acc_panel_gled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[acc_panel[id].acc_panel_gled_port][acc_panel[id].acc_panel_gled_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_gled_port, acc_panel[id].acc_panel_gled_pin, LOG_HIGH);

  //Setup BEEPER
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, acc_panel[id].acc_panel_beep_port, acc_panel[id].acc_panel_beep_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[acc_panel[id].acc_panel_beep_port][acc_panel[id].acc_panel_beep_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_beep_port, acc_panel[id].acc_panel_beep_pin, LOG_HIGH);

  //RELAY
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, acc_panel[id].relay_port, acc_panel[id].relay_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[acc_panel[id].relay_port][acc_panel[id].relay_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].relay_port, acc_panel[id].relay_pin, LOG_HIGH);
}

void panel_deinit(uint8_t id)
{
  if (xTimerDelete(acc_panel[id].timer, 0) != pdFAIL)
  {
    acc_panel[id].timer = NULL;
  }

  weigand_disable(acc_panel[id].acc_panel_port, acc_panel[id].acc_panel_d0_pin, acc_panel[id].acc_panel_d1_pin);
}

int8_t panel_get_request_from_buffer(uint32_t * user_id)
{
  weigand26_buff_item_t item;
  size_t bytes_got;

  // Suspend if empty
  bytes_got = xStreamBufferReceive(_reader_buffer, &item, WEIGAND26_BUFF_ITEM_SIZE, pdMS_TO_TICKS(portMAX_DELAY));

  if (bytes_got == WEIGAND26_BUFF_ITEM_SIZE && weigand_is_parity_ok(item.frame))
  {
    *user_id = (item.frame.facility_code << 16) | item.frame.card_number;
    return item.source;
  }

  return -1;
}

void panel_unlock(uint8_t id, bool with_beep, bool with_ok_led)
{
  // Unlock state
  if (with_beep)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_beep_port, acc_panel[id].acc_panel_beep_pin, LOG_LOW);
  }
  if (with_ok_led)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].acc_panel_gled_port, acc_panel[id].acc_panel_gled_pin, LOG_LOW);
  }
  Chip_GPIO_SetPinState(LPC_GPIO, acc_panel[id].relay_port, acc_panel[id].relay_pin, LOG_LOW);
  //Start timer
  configASSERT(xTimerStart(acc_panel[id].timer, 0));
}

