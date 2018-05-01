#include "main.h"

/**
 * Main program.
 */
int main(void) {
  // Define starting values for global variables.
  uled_state = 0;
  buzzer_state = 0;
  buzzer_tone = 15000;
  buzzer_tone_hz = 15000;
  buzzer_pulses = 500;
  menu_state = TEST_MENU_LED_TOGGLE;
  last_top_row = TEST_MENU_LED_TOGGLE;
  draw_color = 0;

  // Enable the GPIOA clock (buttons on pins A2-A7,
  // user LED on pin A12).
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
  // Enable the GPIOB clock (I2C1 used on pins B6/B7,
  // buzzer on pin B0).
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  // Enable the I2C1 clock.
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
  // Enable the SYSCFG clock for hardware interrupts.
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

  // Setup GPIO pins A2, A3, A4, A5, A6, and A7 as inputs
  // with pullups, low-speed.
  GPIOA->MODER   &= ~(GPIO_MODER_MODER2 |
                      GPIO_MODER_MODER3 |
                      GPIO_MODER_MODER4 |
                      GPIO_MODER_MODER5 |
                      GPIO_MODER_MODER6 |
                      GPIO_MODER_MODER7);
  GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPDR2 |
                      GPIO_PUPDR_PUPDR3 |
                      GPIO_PUPDR_PUPDR4 |
                      GPIO_PUPDR_PUPDR5 |
                      GPIO_PUPDR_PUPDR6 |
                      GPIO_PUPDR_PUPDR7);
  GPIOA->PUPDR   |=  ((1 << GPIO_PUPDR_PUPDR2_Pos) |
                      (1 << GPIO_PUPDR_PUPDR3_Pos) |
                      (1 << GPIO_PUPDR_PUPDR4_Pos) |
                      (1 << GPIO_PUPDR_PUPDR5_Pos) |
                      (1 << GPIO_PUPDR_PUPDR6_Pos) |
                      (1 << GPIO_PUPDR_PUPDR7_Pos));

  // Setup GPIO pin A12 as push-pull output, no pupdr,
  // 10MHz max speed.
  GPIOA->MODER   &= ~(GPIO_MODER_MODER12);
  GPIOA->MODER   |=  (1 << GPIO_MODER_MODER12_Pos);
  GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEEDR12);
  GPIOA->OSPEEDR |=  (1 << GPIO_OSPEEDR_OSPEEDR12_Pos);
  GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT_12);
  GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPDR12);

  // Setup GPIO pin B0 as push-pull output, no pupdr,
  // 10MHz max speed.
  GPIOB->MODER   &= ~(GPIO_MODER_MODER0);
  GPIOB->MODER   |=  (1 << GPIO_MODER_MODER0_Pos);
  GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEEDR0);
  GPIOB->OSPEEDR |=  (1 << GPIO_OSPEEDR_OSPEEDR0_Pos);
  GPIOB->OTYPER  &= ~(GPIO_OTYPER_OT_0);
  GPIOB->PUPDR   &= ~(GPIO_PUPDR_PUPDR0);

  #ifdef VVC_F0
    // Set GPIO pins B6/B7 as alternate function mode 1 for I2C1.
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6);
    GPIOB->AFR[0] |=  (1 << GPIO_AFRL_AFSEL6_Pos);
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL7);
    GPIOB->AFR[0] |=  (1 << GPIO_AFRL_AFSEL7_Pos);
  #elif VVC_F3
    // Set GPIO pins B6/B7 as alternate function mode 4 for I2C1.
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6);
    GPIOB->AFR[0] |=  (4 << GPIO_AFRL_AFSEL6_Pos);
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL7);
    GPIOB->AFR[0] |=  (4 << GPIO_AFRL_AFSEL7_Pos);
  #endif
  // Setup GPIO pins B6, B7 as open-drain alt. func. w/pullup.
  GPIOB->MODER   &= ~(GPIO_MODER_MODER6);
  GPIOB->MODER   |=  (2 << GPIO_MODER_MODER6_Pos);
  GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEEDR6);
  GPIOB->OTYPER  |=  (GPIO_OTYPER_OT_6);
  GPIOB->PUPDR   &= ~(GPIO_PUPDR_PUPDR6);
  GPIOB->PUPDR   |=  (1 << GPIO_PUPDR_PUPDR6_Pos);
  GPIOB->MODER   &= ~(GPIO_MODER_MODER7);
  GPIOB->MODER   |=  (2 << GPIO_MODER_MODER7_Pos);
  GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEEDR7);
  GPIOB->OTYPER  |=  (GPIO_OTYPER_OT_7);
  GPIOB->PUPDR   &= ~(GPIO_PUPDR_PUPDR7);
  GPIOB->PUPDR   |=  (1 << GPIO_PUPDR_PUPDR7_Pos);

  /*
   * Board connectors:
   * A8/A9:        4-pin JST connector 1
   * A10/A11:      4-pin JST connector 2
   * A15/B3/B4/B5: 'GPIO2' 6-pin header; can be used for SPI1.
   * A0/A1/B0:     'GPIO2' 5-pin header.
   * B6/B7:        I2C1 provided on a 4-pin header.
   * Note: Re-assigning pins B6/B7 will disable the screen!
   * The I2C1 bus is provided for communicating with other
   * devices such as sensors, not for GPIO. I2C supports
   * multiple receiving devices on the same line, so long
   * as they have different addresses.
   * (The screen's address is 0x78)
   */

  // Initialize the I2C peripheral and connected devices.
  // (1MHz @ 48MHz PLL)
  i2c_periph_init(I2C1_BASE, 0x50100103);
  // Initialize the SSD1306 OLED display.
  i2c_init_ssd1306(I2C1_BASE);

  // Setup hardware interrupts on the EXTI lines associated
  // with the 6 button inputs.
  // Pins A2, A3 use the EXTI2_3 interrupt.
  // Pins A4, A5, A6, and A7 use the EXTI4_15 interrupt.
  // Map EXTI lines to the GPIOA port. (This is 0x00, but...)
  SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI2);
  SYSCFG->EXTICR[0] |=  (SYSCFG_EXTICR1_EXTI2_PA);
  SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI3);
  SYSCFG->EXTICR[0] |=  (SYSCFG_EXTICR1_EXTI3_PA);
  SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI4);
  SYSCFG->EXTICR[1] |=  (SYSCFG_EXTICR2_EXTI4_PA);
  SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI5);
  SYSCFG->EXTICR[1] |=  (SYSCFG_EXTICR2_EXTI5_PA);
  SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI6);
  SYSCFG->EXTICR[1] |=  (SYSCFG_EXTICR2_EXTI6_PA);
  SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI7);
  SYSCFG->EXTICR[1] |=  (SYSCFG_EXTICR2_EXTI7_PA);
  // Setup the EXTI interrupt lines as 'falling edge' interrupts.
  EXTI->IMR  |=  (EXTI_IMR_MR2);
  EXTI->RTSR &= ~(EXTI_RTSR_TR2);
  EXTI->FTSR |=  (EXTI_FTSR_TR2);
  EXTI->IMR  |=  (EXTI_IMR_MR3);
  EXTI->RTSR &= ~(EXTI_RTSR_TR3);
  EXTI->FTSR |=  (EXTI_FTSR_TR3);
  EXTI->IMR  |=  (EXTI_IMR_MR4);
  EXTI->RTSR &= ~(EXTI_RTSR_TR4);
  EXTI->FTSR |=  (EXTI_FTSR_TR4);
  EXTI->IMR  |=  (EXTI_IMR_MR5);
  EXTI->RTSR &= ~(EXTI_RTSR_TR5);
  EXTI->FTSR |=  (EXTI_FTSR_TR5);
  EXTI->IMR  |=  (EXTI_IMR_MR6);
  EXTI->RTSR &= ~(EXTI_RTSR_TR6);
  EXTI->FTSR |=  (EXTI_FTSR_TR6);
  EXTI->IMR  |=  (EXTI_IMR_MR7);
  EXTI->RTSR &= ~(EXTI_RTSR_TR7);
  EXTI->FTSR |=  (EXTI_FTSR_TR7);

  // The HAL 'cortex' libraries basically just call these
  // core functions for NVIC stuff, anyways:
  #ifdef VVC_F0
    NVIC_SetPriority(EXTI2_3_IRQn, 0x03);
    NVIC_EnableIRQ(EXTI2_3_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 0x03);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
  #elif VVC_F3
    // On Cortex-M4 cores, we need to set an NVIC priority
    // grouping and subpriorities as well as normal priorities:
    // 0x07: 0 Pri / 4 SubPri
    // 0x06: 1 Pri / 3 SubPri
    // 0x05: 2 Pri / 2 SubPri
    // 0x04: 3 Pri / 1 SubPri
    // 0x03: 4 Pri / 0 SubPri
    // Use 2 bits for 'priority' and 2 bits for 'subpriority'.
    NVIC_SetPriorityGrouping(0x05);
    uint32_t exti_pri_encoding = NVIC_EncodePriority(0x05, 0x03, 0x03);
    NVIC_SetPriority(EXTI2_TSC_IRQn, exti_pri_encoding);
    NVIC_EnableIRQ(EXTI2_TSC_IRQn);
    NVIC_SetPriority(EXTI3_IRQn, exti_pri_encoding);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_SetPriority(EXTI4_IRQn, exti_pri_encoding);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_SetPriority(EXTI9_5_IRQn, exti_pri_encoding);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
  #endif

  while (1) {
    draw_test_menu();
    // Communicate the framebuffer to the OLED screen.
    i2c_display_framebuffer(I2C1_BASE, &oled_fb);

    // Set the onboard LED.
    if (uled_state) {
      GPIOA->ODR |=  (GPIO_ODR_12);
    }
    else {
      GPIOA->ODR &= ~(GPIO_ODR_12);
    }

    // Play a tone on the buzzer if applicable.
    if (buzzer_state) {
      // This probably is not actually an accurate conversion
      // to Hertz. But it's a 48MHz clock so...
      buzzer_tone = 48000000 / buzzer_tone_hz;
      // Play for about 1/8-second. So:
      // X pulses * Y cycles = 6000000
      buzzer_pulses = 6000000 / buzzer_tone;
      pulse_out_pin(&GPIOB->ODR,
                    GPIO_ODR_0,
                    buzzer_tone,
                    buzzer_pulses);
      buzzer_state = 0;
    }
  }
  return 0;
}