#include "peripherals.h"

void start_timer(TIM_TypeDef *TIMx,
                 uint16_t prescaler,
                 uint16_t period) {
  // Initialize the 'TIM2' timer peripheral. (TODO: F303 support)
  // Start by making sure the timer's 'counter' is off.
  TIMx->CR1 &= ~(TIM_CR1_CEN);
  // Next, reset the peripheral. (This is where a HAL can help)
  if (TIMx == TIM2) {
    RCC->APB1RSTR |=  (RCC_APB1RSTR_TIM2RST);
    RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM2RST);
  }
  // Set clock division to 1; the timer triggers every N events.
  // Also set the counter to count up.
  TIMx->CR1  &= ~(TIM_CR1_DIR |
                  TIM_CR1_CKD);
  // Set the 'prescaler'; the timer 'ticks' every N clock cycles.
  TIMx->PSC   =  prescaler;
  // Set 'auto-reload'; this is the timer's period in 'ticks'.
  TIMx->ARR   =  period;
  // Set the repetition counter to 0; I dunno, unused for now.
  TIMx->RCR   =  0;
  // Before enabling the counter, trigger an 'update' event to
  // make sure that the chip registers the 'prescaler' settings.
  TIMx->EGR  |=  (TIM_EGR_UG);
  // Enable the 'update' timer event/interrupt.
  TIMx->DIER |=  (TIM_DIER_UIE);
  // Finally, enable the timer.
  TIM2->CR1  |=  (TIM_CR1_CEN);
}

void stop_timer(TIM_TypeDef *TIMx) {
  // Turn off the timer's 'counter'.
  TIM2->CR1 &= ~(TIM_CR1_CEN);
  // Clear the 'pending update interrupt' flag.
  TIM2->SR  &= ~(TIM_SR_UIF);
}
