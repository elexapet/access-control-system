
#include <reader.h>

static StreamBufferHandle_t _reader_buffer;

static const reader_wiring_t _reader_wiring[ACS_READER_MAXCOUNT] =
{
  {
    .data_port = ACS_READER_A_DATA_PORT,
    .d0_pin = ACS_READER_A_D0_PIN,
    .d1_pin = ACS_READER_A_D1_PIN,
    .beep_port = ACS_READER_A_BEEP_PORT,
    .beep_pin = ACS_READER_A_BEEP_PIN,
    .gled_port = ACS_READER_A_GLED_PORT,
    .gled_pin = ACS_READER_A_GLED_PIN,
    .rled_port = ACS_READER_A_RLED_PORT,
    .rled_pin = ACS_READER_A_RLED_PIN,
    .relay_port = ACS_READER_A_RELAY_PORT,
    .relay_pin = ACS_READER_A_RELAY_PIN,
    .sensor_port = ACS_READER_A_SENSOR_PORT,
    .sensor_pin = ACS_READER_A_SENSOR_PIN,
  },
  {
    .data_port = ACS_READER_B_DATA_PORT,
    .d0_pin = ACS_READER_B_D0_PIN,
    .d1_pin = ACS_READER_B_D1_PIN,
    .beep_port = ACS_READER_B_BEEP_PORT,
    .beep_pin = ACS_READER_B_BEEP_PIN,
    .gled_port = ACS_READER_B_GLED_PORT,
    .gled_pin = ACS_READER_B_GLED_PIN,
    .rled_port = ACS_READER_B_RLED_PORT,
    .rled_pin = ACS_READER_B_RLED_PIN,
    .relay_port = ACS_READER_B_RELAY_PORT,
    .relay_pin = ACS_READER_B_RELAY_PIN,
    .sensor_port = ACS_READER_B_SENSOR_PORT,
    .sensor_pin = ACS_READER_B_SENSOR_PIN,
  }
};

reader_conf_t reader_conf[ACS_READER_MAXCOUNT] =
{
  {
    .timer_ok = NULL,
    .timer_open = NULL,
    .open_time_sec = ACS_READER_A_OPEN_TIME_MS,
    .gled_time_sec = ACS_READER_A_OK_GLED_TIME_MS,
    .enabled = ACS_READER_A_ENABLED,
  },
  {
    .timer_ok = NULL,
    .timer_open = NULL,
    .open_time_sec = ACS_READER_B_OPEN_TIME_MS,
    .gled_time_sec = ACS_READER_B_OK_GLED_TIME_MS,
    .enabled = ACS_READER_B_ENABLED,
  }
};


static void reader_timer_open_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id < ACS_READER_COUNT)
  {
    // Lock state
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].relay_port, _reader_wiring[id].relay_pin, LOG_HIGH);
  }
}

static void reader_timer_ok_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id < ACS_READER_COUNT)
  {
    // Lock state
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].beep_port, _reader_wiring[id].beep_pin, LOG_LOW);
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].gled_port, _reader_wiring[id].gled_pin, LOG_LOW);
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].rled_port, _reader_wiring[id].rled_pin, LOG_HIGH);
  }
}


void reader_init(uint8_t id)
{
  //Create stream buffer to receive id from all card readers
  if (_reader_buffer == NULL)
  {
    _reader_buffer = xStreamBufferCreate(2 * ACS_READER_COUNT * WEIGAND26_BUFF_ITEM_SIZE, WEIGAND26_BUFF_ITEM_SIZE);
  }
  configASSERT(_reader_buffer);

  //Init interface to reader
  weigand_init(_reader_buffer, id, _reader_wiring[id].data_port, _reader_wiring[id].d0_pin, _reader_wiring[id].d1_pin);

  //Create timer only once
  if (reader_conf[id].timer_open == NULL)
  {
    reader_conf[id].timer_open = xTimerCreate("PT0", (reader_conf[id].open_time_sec / portTICK_PERIOD_MS), pdFALSE, (void *)(uint32_t) id, reader_timer_open_callback);
  }
  configASSERT(reader_conf[id].timer_open);

  if (reader_conf[id].timer_ok == NULL)
  {
    reader_conf[id].timer_ok = xTimerCreate("PT1", (reader_conf[id].gled_time_sec / portTICK_PERIOD_MS), pdFALSE, (void *)(uint32_t) id, reader_timer_ok_callback);
  }
  configASSERT(reader_conf[id].timer_ok);

  //Setup GLED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[id].gled_port, _reader_wiring[id].gled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[id].gled_port][_reader_wiring[id].gled_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].gled_port, _reader_wiring[id].gled_pin, LOG_LOW);

  //Setup RLED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[id].rled_port, _reader_wiring[id].rled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[id].rled_port][_reader_wiring[id].rled_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].rled_port, _reader_wiring[id].rled_pin, LOG_HIGH);

  //Setup BEEPER
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[id].beep_port, _reader_wiring[id].beep_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[id].beep_port][_reader_wiring[id].beep_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].beep_port, _reader_wiring[id].beep_pin, LOG_LOW);

  //RELAY
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[id].relay_port, _reader_wiring[id].relay_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[id].relay_port][_reader_wiring[id].relay_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].relay_port, _reader_wiring[id].relay_pin, LOG_HIGH);

  //SENSOR
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[id].sensor_port][_reader_wiring[id].sensor_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, _reader_wiring[id].sensor_port, _reader_wiring[id].sensor_pin);
#ifdef SENSOR_TYPE
#if SENSOR_TYPE == SENSOR_IS_NO
  Chip_GPIO_SetupPinInt(LPC_GPIO, _reader_wiring[id].sensor_port, _reader_wiring[id].sensor_pin, GPIO_INT_ACTIVE_LOW_LEVEL);
#else
  Chip_GPIO_SetupPinInt(LPC_GPIO, _reader_wiring[id].sensor_port, _reader_wiring[id].sensor_pin, GPIO_INT_ACTIVE_HIGH_LEVEL);
  Chip_GPIO_ClearInts(LPC_GPIO, _reader_wiring[id].sensor_port, (1 << _reader_wiring[id].sensor_pin));
  Chip_GPIO_EnableInt(LPC_GPIO, _reader_wiring[id].sensor_port, (1 << _reader_wiring[id].sensor_pin));
#endif
#endif
}

void reader_deinit(uint8_t id)
{
  if (xTimerDelete(reader_conf[id].timer_open, 0) != pdFAIL)
  {
    reader_conf[id].timer_open = NULL;
  }
  if (xTimerDelete(reader_conf[id].timer_ok, 0) != pdFAIL)
  {
    reader_conf[id].timer_ok = NULL;
  }

  weigand_disable(_reader_wiring[id].data_port, _reader_wiring[id].d0_pin, _reader_wiring[id].d1_pin);
}

int8_t reader_get_request_from_buffer(uint32_t * user_id)
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

void reader_unlock(uint8_t id, bool with_beep, bool with_ok_led)
{
  // Unlock state
  if (with_beep)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].beep_port, _reader_wiring[id].beep_pin, LOG_HIGH);
  }
  if (with_ok_led)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].gled_port, _reader_wiring[id].gled_pin, LOG_HIGH);
  }
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].rled_port, _reader_wiring[id].rled_pin, LOG_LOW);
  configASSERT(xTimerStart(reader_conf[id].timer_open, 0));

  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].relay_port, _reader_wiring[id].relay_pin, LOG_LOW);
  configASSERT(xTimerStart(reader_conf[id].timer_ok, 0));
}

//GPIO port 0 handler
void PIOINT0_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(EINT0_IRQn);
  uint32_t int_states = Chip_GPIO_GetMaskedInts(LPC_GPIO, 0);
  //Clear int flag on each pin
  Chip_GPIO_ClearInts(LPC_GPIO, 0, 0xFFFFFFFF);

  if (int_states & (1 << _reader_wiring[ACS_READER_A_IDX].sensor_pin))
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[ACS_READER_A_IDX].relay_port, _reader_wiring[ACS_READER_A_IDX].relay_pin, LOG_HIGH);
  }
  else if (int_states & (1 << _reader_wiring[ACS_READER_B_IDX].sensor_pin))
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[ACS_READER_B_IDX].relay_port, _reader_wiring[ACS_READER_B_IDX].relay_pin, LOG_HIGH);
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
