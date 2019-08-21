
#include "panel.h"

static StreamBufferHandle_t _reader_buffer;

static const panel_wiring_t _panel_wiring[DOOR_ACC_PANEL_MAXCOUNT] =
{
  {
    .acc_panel_port = DOOR_1_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_1_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_1_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_1_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_1_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_1_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_1_ACC_PANEL_GLED_PIN,
    .acc_panel_rled_port = DOOR_1_ACC_PANEL_RLED_PORT,
    .acc_panel_rled_pin = DOOR_1_ACC_PANEL_RLED_PIN,
    .relay_port = DOOR_1_RELAY_PORT,
    .relay_pin = DOOR_1_RELAY_PIN,
    .sensor_port = DOOR_1_SENSOR_PORT,
    .sensor_pin = DOOR_1_SENSOR_PIN,
  },
  {
    .acc_panel_port = DOOR_2_ACC_PANEL_PORT,
    .acc_panel_d0_pin = DOOR_2_ACC_PANEL_D0_PIN,
    .acc_panel_d1_pin = DOOR_2_ACC_PANEL_D1_PIN,
    .acc_panel_beep_port = DOOR_2_ACC_PANEL_BEEP_PORT,
    .acc_panel_beep_pin = DOOR_2_ACC_PANEL_BEEP_PIN,
    .acc_panel_gled_port = DOOR_2_ACC_PANEL_GLED_PORT,
    .acc_panel_gled_pin = DOOR_2_ACC_PANEL_GLED_PIN,
    .acc_panel_rled_port = DOOR_2_ACC_PANEL_RLED_PORT,
    .acc_panel_rled_pin = DOOR_2_ACC_PANEL_RLED_PIN,
    .relay_port = DOOR_2_RELAY_PORT,
    .relay_pin = DOOR_2_RELAY_PIN,
    .sensor_port = DOOR_2_SENSOR_PORT,
    .sensor_pin = DOOR_2_SENSOR_PIN,
  }
};

panel_conf_t panel_conf[DOOR_ACC_PANEL_MAXCOUNT] =
{
  {
    .timer_ok = NULL,
    .timer_open = NULL,
    .open_time_sec = DOOR_1_OPEN_TIME_MS,
    .gled_time_sec = DOOR_1_OK_GLED_TIME_MS,
    .mode = PANEL_MODE_DEF,
    .enabled = DOOR_1_ACC_PANEL_ON,
  },
  {
    .timer_ok = NULL,
    .timer_open = NULL,
    .open_time_sec = DOOR_2_OPEN_TIME_MS,
    .gled_time_sec = DOOR_2_OK_GLED_TIME_MS,
    .mode = PANEL_MODE_DEF,
    .enabled = DOOR_2_ACC_PANEL_ON,
  }
};


static void panel_timer_open_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id < DOOR_ACC_PANEL_COUNT)
  {
    // Lock state
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].relay_port, _panel_wiring[id].relay_pin, LOG_HIGH);
  }
}

static void panel_timer_ok_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id < DOOR_ACC_PANEL_COUNT)
  {
    // Lock state
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_beep_port, _panel_wiring[id].acc_panel_beep_pin, LOG_LOW);
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_gled_port, _panel_wiring[id].acc_panel_gled_pin, LOG_LOW);
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_rled_port, _panel_wiring[id].acc_panel_rled_pin, LOG_HIGH);
  }
}


void panel_init(uint8_t id)
{
  //Create stream buffer to receive id from all card readers
  if (_reader_buffer == NULL)
  {
    _reader_buffer = xStreamBufferCreate(2 * DOOR_ACC_PANEL_COUNT * WEIGAND26_BUFF_ITEM_SIZE, WEIGAND26_BUFF_ITEM_SIZE);
  }
  configASSERT(_reader_buffer);

  //Init interface to reader
  weigand_init(_reader_buffer, id, _panel_wiring[id].acc_panel_port, _panel_wiring[id].acc_panel_d0_pin, _panel_wiring[id].acc_panel_d1_pin);

  //Create timer only once
  if (panel_conf[id].timer_open == NULL)
  {
    panel_conf[id].timer_open = xTimerCreate("PT0", (panel_conf[id].open_time_sec / portTICK_PERIOD_MS), pdFALSE, (void *)(uint32_t) id, panel_timer_open_callback);
  }
  configASSERT(panel_conf[id].timer_open);

  if (panel_conf[id].timer_ok == NULL)
  {
    panel_conf[id].timer_ok = xTimerCreate("PT1", (panel_conf[id].gled_time_sec / portTICK_PERIOD_MS), pdFALSE, (void *)(uint32_t) id, panel_timer_ok_callback);
  }
  configASSERT(panel_conf[id].timer_ok);

  //Setup GLED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _panel_wiring[id].acc_panel_gled_port, _panel_wiring[id].acc_panel_gled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_panel_wiring[id].acc_panel_gled_port][_panel_wiring[id].acc_panel_gled_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_gled_port, _panel_wiring[id].acc_panel_gled_pin, LOG_LOW);

  //Setup RLED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _panel_wiring[id].acc_panel_rled_port, _panel_wiring[id].acc_panel_rled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_panel_wiring[id].acc_panel_rled_port][_panel_wiring[id].acc_panel_rled_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_rled_port, _panel_wiring[id].acc_panel_rled_pin, LOG_HIGH);

  //Setup BEEPER
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _panel_wiring[id].acc_panel_beep_port, _panel_wiring[id].acc_panel_beep_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_panel_wiring[id].acc_panel_beep_port][_panel_wiring[id].acc_panel_beep_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_beep_port, _panel_wiring[id].acc_panel_beep_pin, LOG_LOW);

  //RELAY
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _panel_wiring[id].relay_port, _panel_wiring[id].relay_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_panel_wiring[id].relay_port][_panel_wiring[id].relay_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].relay_port, _panel_wiring[id].relay_pin, LOG_HIGH);

  //SENSOR
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_panel_wiring[id].sensor_port][_panel_wiring[id].sensor_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, _panel_wiring[id].sensor_port, _panel_wiring[id].sensor_pin);
#ifdef SENSOR_TYPE
#if SENSOR_TYPE == SENSOR_IS_NO
  Chip_GPIO_SetupPinInt(LPC_GPIO, _panel_wiring[id].sensor_port, _panel_wiring[id].sensor_pin, GPIO_INT_ACTIVE_LOW_LEVEL);
#else
  Chip_GPIO_SetupPinInt(LPC_GPIO, _panel_wiring[id].sensor_port, _panel_wiring[id].sensor_pin, GPIO_INT_ACTIVE_HIGH_LEVEL);
  Chip_GPIO_ClearInts(LPC_GPIO, _panel_wiring[id].sensor_port, (1 << _panel_wiring[id].sensor_pin));
  Chip_GPIO_EnableInt(LPC_GPIO, _panel_wiring[id].sensor_port, (1 << _panel_wiring[id].sensor_pin));
#endif
#endif
}

void panel_deinit(uint8_t id)
{
  if (xTimerDelete(panel_conf[id].timer_open, 0) != pdFAIL)
  {
    panel_conf[id].timer_open = NULL;
  }
  if (xTimerDelete(panel_conf[id].timer_ok, 0) != pdFAIL)
  {
    panel_conf[id].timer_ok = NULL;
  }

  weigand_disable(_panel_wiring[id].acc_panel_port, _panel_wiring[id].acc_panel_d0_pin, _panel_wiring[id].acc_panel_d1_pin);
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
  else
  {
    return -1;
  }
}

void panel_unlock(uint8_t id, bool with_beep, bool with_ok_led)
{
  // Unlock state
  if (with_beep)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_beep_port, _panel_wiring[id].acc_panel_beep_pin, LOG_HIGH);
  }
  if (with_ok_led)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_gled_port, _panel_wiring[id].acc_panel_gled_pin, LOG_HIGH);
  }
  Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].acc_panel_rled_port, _panel_wiring[id].acc_panel_rled_pin, LOG_LOW);
  configASSERT(xTimerStart(panel_conf[id].timer_open, 0));

  Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[id].relay_port, _panel_wiring[id].relay_pin, LOG_LOW);
  configASSERT(xTimerStart(panel_conf[id].timer_ok, 0));
}

//GPIO port 0 handler
void PIOINT0_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(EINT0_IRQn);
  uint32_t int_states = Chip_GPIO_GetMaskedInts(LPC_GPIO, 0);
  //Clear int flag on each pin
  Chip_GPIO_ClearInts(LPC_GPIO, 0, 0xFFFFFFFF);

  if (int_states & (1 << _panel_wiring[ACC_PANEL_A].sensor_pin))
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[ACC_PANEL_A].relay_port, _panel_wiring[ACC_PANEL_A].relay_pin, LOG_HIGH);
  }
  else if (int_states & (1 << _panel_wiring[ACC_PANEL_B].sensor_pin))
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _panel_wiring[ACC_PANEL_B].relay_port, _panel_wiring[ACC_PANEL_B].relay_pin, LOG_HIGH);
  }
  else
  {
    return;
  }
}

//GPIO port 1 handler
void PIOINT1_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(EINT1_IRQn);

}
