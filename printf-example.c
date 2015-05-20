#ifndef __NO_SYSTEM_INIT
void SystemInit()
{}
#endif

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

uint32_t  ctr[4]={0,0,0,0};
uint32_t  interval[4]={2203,971,503,4969};
uint32_t  port[4]={GPIOB,GPIOB,GPIOB,GPIOB};
uint16_t  portbit[4]={GPIO12,GPIO13,GPIO14,GPIO15};
int       systick_i;

/// Note: USB serial doodah for GPS:
//  BLK = GND
//  RED = 5V
//  YEL = TXD
//  GRN = RXI
// /usr/share/gcc-arm-none-eabi/samples

static void gpio_setup(void)
{

	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO12|GPIO13|GPIO14|GPIO15);

	gpio_set_mode(GPIOC, GPIO_MODE_INPUT,
		      GPIO_CNF_INPUT_PULL_UPDOWN, GPIO6|GPIO7|GPIO8);
		      
  gpio_set(GPIOC,GPIO6|GPIO7|GPIO8);
  
}

static void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_24mhz();

	/* Enable GPIOC clock. */
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOB);

}

void timer_setup(void) {

  // set systick clock source
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
  // set autoreload counter
  systick_set_reload(1499);
  // enable systick interrupt
  systick_interrupt_enable();
  // GO
  systick_counter_enable();
  
}

void SysTick_Handler(void) {

  for ( systick_i=0 ; systick_i<4 ; systick_i++ ) {
    if (ctr[systick_i]++ == interval[systick_i]) {
      ctr[systick_i]=0;
      gpio_toggle(port[systick_i],portbit[systick_i]);
    }
  }
}
    
void main() {

  clock_setup();
  gpio_setup();
  timer_setup();
  
  while (1);
}

/* Systick interrupts: */


