#include "gimbal_keys.h"
#include "adc.h"

#define GKEY_DEBOUNCE_MS       20U
#define GKEY_LONG_MS          800U
#define GKEY_ZERO_MS         3000U
#define GKEY_VERY_LONG_MS    5000U
#define GKEY_QUEUE_LEN          8U

extern uint16_t adc_val[2];

typedef struct {
  GimbalKeyId stable;
  GimbalKeyId raw_last;
  uint32_t raw_changed_ms;
  uint32_t press_start_ms;
  uint8_t long_sent;
  uint8_t zero_sent;
  uint8_t very_long_sent;
  uint8_t held_used;
} GimbalKeyScanner;

static GimbalKeyScanner scanner;
static GimbalKeyEvent queue[GKEY_QUEUE_LEN];
static uint8_t q_head;
static uint8_t q_tail;

static void sample_key_adc(void)
{
  if (HAL_ADC_Start(&hadc1) != HAL_OK) {
    return;
  }

  if (HAL_ADC_PollForConversion(&hadc1, 1U) == HAL_OK) {
    adc_val[0] = (uint16_t)HAL_ADC_GetValue(&hadc1);
  }

  if (HAL_ADC_PollForConversion(&hadc1, 1U) == HAL_OK) {
    adc_val[1] = (uint16_t)HAL_ADC_GetValue(&hadc1);
  }

  (void)HAL_ADC_Stop(&hadc1);
}

static GimbalKeyId key_from_adc(uint16_t value)
{
  if (value < 200U) {
    return GKEY_CENTER;
  }
  if (value > 700U && value < 1000U) {
    return GKEY_DOWN;
  }
  if (value > 1500U && value < 1800U) {
    return GKEY_UP;
  }
  if (value > 2200U && value < 2500U) {
    return GKEY_RIGHT;
  }
  if (value > 2800U && value < 3500U) {
    return GKEY_LEFT;
  }
  return GKEY_NONE;
}

static void push_event(GimbalKeyId key, GimbalKeyEventType type)
{
  uint8_t next = (uint8_t)((q_head + 1U) % GKEY_QUEUE_LEN);
  if (next == q_tail) {
    q_tail = (uint8_t)((q_tail + 1U) % GKEY_QUEUE_LEN);
  }
  queue[q_head].key = key;
  queue[q_head].type = type;
  q_head = next;
}

void GimbalKeys_Init(void)
{
  scanner.stable = GKEY_NONE;
  scanner.raw_last = GKEY_NONE;
  scanner.raw_changed_ms = HAL_GetTick();
  scanner.press_start_ms = 0U;
  scanner.long_sent = 0U;
  scanner.very_long_sent = 0U;
  q_head = 0U;
  q_tail = 0U;
}

void GimbalKeys_Task(uint32_t now_ms)
{
  sample_key_adc();
  GimbalKeyId raw = key_from_adc(adc_val[1]);

  if (raw != scanner.raw_last) {
    scanner.raw_last = raw;
    scanner.raw_changed_ms = now_ms;
  }

  if ((now_ms - scanner.raw_changed_ms) < GKEY_DEBOUNCE_MS) {
    return;
  }

  if (scanner.stable != raw) {
    GimbalKeyId old = scanner.stable;
    scanner.stable = raw;
    if (raw != GKEY_NONE) {
      scanner.press_start_ms = now_ms;
      scanner.long_sent = 0U;
      scanner.zero_sent = 0U;
      scanner.very_long_sent = 0U;
      scanner.held_used = 0U;
    } else if (old != GKEY_NONE) {
      if (scanner.long_sent == 0U && scanner.very_long_sent == 0U && scanner.held_used == 0U) {
        push_event(old, GKEY_EVENT_SHORT);
      }
    }
    return;
  }

  if (scanner.stable != GKEY_NONE) {
    uint32_t held = now_ms - scanner.press_start_ms;
    if (held >= GKEY_VERY_LONG_MS && scanner.very_long_sent == 0U) {
      scanner.very_long_sent = 1U;
      scanner.zero_sent = 1U;
      scanner.long_sent = 1U;
      push_event(scanner.stable, GKEY_EVENT_VERY_LONG);
    } else if (held >= GKEY_ZERO_MS && scanner.zero_sent == 0U) {
      scanner.zero_sent = 1U;
      scanner.long_sent = 1U;
      push_event(scanner.stable, GKEY_EVENT_ZERO);
    } else if (held >= GKEY_LONG_MS && scanner.long_sent == 0U) {
      scanner.long_sent = 1U;
      push_event(scanner.stable, GKEY_EVENT_LONG);
    }
  }
}

uint8_t GimbalKeys_GetEvent(GimbalKeyEvent *event)
{
  if (q_head == q_tail || event == 0) {
    return 0U;
  }
  *event = queue[q_tail];
  q_tail = (uint8_t)((q_tail + 1U) % GKEY_QUEUE_LEN);
  return 1U;
}

GimbalKeyId GimbalKeys_GetHeldKey(void)
{
  return scanner.stable;
}

uint32_t GimbalKeys_GetHeldMs(uint32_t now_ms)
{
  if (scanner.stable == GKEY_NONE) {
    return 0U;
  }
  return now_ms - scanner.press_start_ms;
}

void GimbalKeys_MarkHeldUsed(void)
{
  if (scanner.stable != GKEY_NONE) {
    scanner.held_used = 1U;
  }
}

uint16_t GimbalKeys_GetAdcRaw(void)
{
  return adc_val[1];
}
