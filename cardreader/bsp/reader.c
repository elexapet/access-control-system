/**
 *  @file
 *  @brief RFID reader driver
 *
 *         This RFID reader implementation uses Weigand26 card reader protocol.
 *
 *  @author Petr Elexa
 *  @see LICENSE
 *
 */

#include <reader.h>

// Define sensor binary value when door is open.
#if DOOR_SENSOR_TYPE == SENSOR_IS_NC
#define DOOR_SENSOR_VALUE_OPEN LOG_LOW  // Switch is closed when door is open.
#else
#define DOOR_SENSOR_VALUE_OPEN LOG_HIGH // Switch is open when door is open.
#endif

#define DOOR_OPEN 1
#define DOOR_CLOSED 0

// Buffer for user_id received from RFID reader.
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
    .door_open = DOOR_CLOSED
  },
  {
    .timer_ok = NULL,
    .timer_open = NULL,
    .open_time_sec = ACS_READER_B_OPEN_TIME_MS,
    .gled_time_sec = ACS_READER_B_OK_GLED_TIME_MS,
    .enabled = ACS_READER_B_ENABLED,
    .door_open = DOOR_CLOSED
  }
};


// Timer callback to lock after specified time
static void _timer_open_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id < ACS_READER_MAXCOUNT && reader_conf[id].enabled)
  {
    // Lock state
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].relay_port, _reader_wiring[id].relay_pin, LOG_HIGH);
  }
}

// Timer callback to stop signaling unlock
static void _timer_ok_callback(TimerHandle_t pxTimer)
{
  configASSERT(pxTimer);

  // Which timer expired
  uint32_t id = (uint32_t) pvTimerGetTimerID(pxTimer);

  if (id < ACS_READER_MAXCOUNT && reader_conf[id].enabled)
  {
    // Lock state
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].beep_port, _reader_wiring[id].beep_pin, LOG_LOW);
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].gled_port, _reader_wiring[id].gled_pin, LOG_LOW);
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[id].rled_port, _reader_wiring[id].rled_pin, LOG_HIGH);
  }
}


void reader_init(uint8_t idx)
{
  //Create stream buffer to receive idx from all card readers
  if (_reader_buffer == NULL)
  {
    _reader_buffer = xStreamBufferCreate(2 * ACS_READER_MAXCOUNT * WEIGAND26_BUFF_ITEM_SIZE, WEIGAND26_BUFF_ITEM_SIZE);
  }
  configASSERT(_reader_buffer);

  //Init interface to reader
  weigand_init(_reader_buffer, idx, _reader_wiring[idx].data_port, _reader_wiring[idx].d0_pin, _reader_wiring[idx].d1_pin);

  //Create timer only once
  if (reader_conf[idx].timer_open == NULL)
  {
    reader_conf[idx].timer_open = xTimerCreate("PT0", pdMS_TO_TICKS(reader_conf[idx].open_time_sec), pdFALSE, (void *)(uint32_t) idx, _timer_open_callback);
  }
  configASSERT(reader_conf[idx].timer_open);

  if (reader_conf[idx].timer_ok == NULL)
  {
    reader_conf[idx].timer_ok = xTimerCreate("PT1", pdMS_TO_TICKS(reader_conf[idx].gled_time_sec), pdFALSE, (void *)(uint32_t) idx, _timer_ok_callback);
  }
  configASSERT(reader_conf[idx].timer_ok);

  //Setup GLED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[idx].gled_port, _reader_wiring[idx].gled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[idx].gled_port][_reader_wiring[idx].gled_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].gled_port, _reader_wiring[idx].gled_pin, LOG_LOW);

  //Setup RLED
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[idx].rled_port, _reader_wiring[idx].rled_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[idx].rled_port][_reader_wiring[idx].rled_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].rled_port, _reader_wiring[idx].rled_pin, LOG_HIGH);

  //Setup BEEPER
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[idx].beep_port, _reader_wiring[idx].beep_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[idx].beep_port][_reader_wiring[idx].beep_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].beep_port, _reader_wiring[idx].beep_pin, LOG_LOW);

  //RELAY
  Chip_GPIO_SetPinDIROutput(LPC_GPIO, _reader_wiring[idx].relay_port, _reader_wiring[idx].relay_pin);
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[idx].relay_port][_reader_wiring[idx].relay_pin], IOCON_MODE_PULLUP, IOCON_FUNC0);
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].relay_port, _reader_wiring[idx].relay_pin, LOG_HIGH);

  //SENSOR (has pull-up resistor)
  Chip_IOCON_PinMux(LPC_IOCON, CHIP_IOCON_PIO[_reader_wiring[idx].sensor_port][_reader_wiring[idx].sensor_pin], IOCON_MODE_INACT, IOCON_FUNC0);
  Chip_GPIO_SetPinDIRInput(LPC_GPIO, _reader_wiring[idx].sensor_port, _reader_wiring[idx].sensor_pin);
#ifdef DOOR_SENSOR_TYPE
  Chip_GPIO_SetupPinInt(LPC_GPIO, _reader_wiring[idx].sensor_port, _reader_wiring[idx].sensor_pin, GPIO_INT_BOTH_EDGES);
  Chip_GPIO_ClearInts(LPC_GPIO, _reader_wiring[idx].sensor_port, (1 << _reader_wiring[idx].sensor_pin));
  Chip_GPIO_EnableInt(LPC_GPIO, _reader_wiring[idx].sensor_port, (1 << _reader_wiring[idx].sensor_pin));
#endif
}

void reader_deinit(uint8_t idx)
{
  if (xTimerDelete(reader_conf[idx].timer_open, 0) != pdFAIL)
  {
    reader_conf[idx].timer_open = NULL;
  }
  if (xTimerDelete(reader_conf[idx].timer_ok, 0) != pdFAIL)
  {
    reader_conf[idx].timer_ok = NULL;
  }

  weigand_disable(_reader_wiring[idx].data_port, _reader_wiring[idx].d0_pin, _reader_wiring[idx].d1_pin);
}

uint8_t reader_get_request_from_buffer(uint32_t * user_id, uint16_t time_to_wait_ms)
{
  weigand26_buff_item_t item;
  size_t bytes_got;

  // Suspend if empty
  bytes_got = xStreamBufferReceive(_reader_buffer, &item, WEIGAND26_BUFF_ITEM_SIZE, pdMS_TO_TICKS(time_to_wait_ms));

  if (bytes_got == WEIGAND26_BUFF_ITEM_SIZE && weigand_is_parity_ok(item.frame))
  {
    *user_id = (item.frame.facility_code << 16) | item.frame.card_number;
    return item.source;
  }
  else
  {
    return UINT8_MAX;
  }
}

void reader_unlock(uint8_t idx, bool with_beep, bool with_ok_led)
{
  // Unlock state
  if (with_beep)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].beep_port, _reader_wiring[idx].beep_pin, LOG_HIGH);
  }
  if (with_ok_led)
  {
    Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].gled_port, _reader_wiring[idx].gled_pin, LOG_HIGH);
  }
  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].rled_port, _reader_wiring[idx].rled_pin, LOG_LOW);
  configASSERT(xTimerStart(reader_conf[idx].timer_open, 0));

  Chip_GPIO_SetPinState(LPC_GPIO, _reader_wiring[idx].relay_port, _reader_wiring[idx].relay_pin, LOG_LOW);
  configASSERT(xTimerStart(reader_conf[idx].timer_ok, 0));
}

bool reader_is_door_open(uint8_t reader_idx)
{
  return reader_conf[reader_idx].door_open == DOOR_OPEN;
}

// Sensor interrupt handler.
void reader_sensor_int_handler(uint8_t port, uint32_t int_states)
{
  if (_reader_wiring[ACS_READER_A_IDX].sensor_port != port &&
      _reader_wiring[ACS_READER_B_IDX].sensor_port != port)
  {
    return;
  }

  // door sensor A
  if (int_states & (1 << _reader_wiring[ACS_READER_A_IDX].sensor_pin))
  {
    // update current state
    uint8_t sensor_value = Chip_GPIO_ReadPortBit(LPC_GPIO, port, _reader_wiring[ACS_READER_A_IDX].sensor_pin);
    reader_conf[ACS_READER_A_IDX].door_open = (sensor_value == DOOR_SENSOR_VALUE_OPEN ? DOOR_OPEN : DOOR_CLOSED);
  }
  // door sensor B
  if (int_states & (1 << _reader_wiring[ACS_READER_B_IDX].sensor_pin))
  {
    // update current state
    uint8_t sensor_value = Chip_GPIO_ReadPortBit(LPC_GPIO, port, _reader_wiring[ACS_READER_B_IDX].sensor_pin);
    reader_conf[ACS_READER_B_IDX].door_open = (sensor_value == DOOR_SENSOR_VALUE_OPEN ? DOOR_OPEN : DOOR_CLOSED);
  }
}

// GPIO port 0 handler.
void PIOINT0_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(EINT0_IRQn);
  uint32_t int_states = Chip_GPIO_GetMaskedInts(LPC_GPIO, 0);
  //Clear int flag on each pin
  Chip_GPIO_ClearInts(LPC_GPIO, 0, 0xFFFFFFFF);

  reader_sensor_int_handler(0, int_states);
}

// GPIO port 1 handler.
void PIOINT1_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(EINT1_IRQn);
  uint32_t int_states = Chip_GPIO_GetMaskedInts(LPC_GPIO, 1);
  //Clear int flag on each pin
  Chip_GPIO_ClearInts(LPC_GPIO, 1, 0xFFFFFFFF);

  reader_sensor_int_handler(1, int_states);
}
