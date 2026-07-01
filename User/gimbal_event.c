#include "gimbal_event.h"
#include "main.h"

static volatile uint32_t g_event_flags;

void GimbalEvent_Init(void)
{
  __disable_irq();
  g_event_flags = 0U;
  __enable_irq();
}

void GimbalEvent_Push(uint32_t events)
{
  __disable_irq();
  g_event_flags |= events;
  __enable_irq();
}

void GimbalEvent_PushFromIsr(uint32_t events)
{
  g_event_flags |= events;
}

uint8_t GimbalEvent_Fetch(uint32_t *events)
{
  uint32_t pending;

  if (events == 0) {
    return 0U;
  }

  __disable_irq();
  pending = g_event_flags;
  g_event_flags = 0U;
  __enable_irq();

  *events = pending;
  return (pending != 0U) ? 1U : 0U;
}

uint32_t GimbalEvent_Peek(void)
{
  return g_event_flags;
}
