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

  /* STEP 1 - Enable IO clock for IOPB */
  /* Ref: RM0008 p */
  
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO12|GPIO13|GPIO14|GPIO15);

	gpio_set_mode(GPIOC, GPIO_MODE_INPUT,
		      GPIO_CNF_INPUT_PULL_UPDOWN, GPIO6|GPIO7|GPIO8);
		      
  gpio_set(GPIOC,GPIO6|GPIO7|GPIO8);
  
}

static void clock_setup(void)
{

  /* Set clock to 24MHz */
  /* Reg names: /usr/local/arm-none-eabi/include/libopencm3/stm32/f1/rcc.h */
  /* On reset, clock source = HSI */
  
  // STEP 1: Turn on HSE
  // ref RM0008 p90
  
  RCC_CR |= RCC_CR_HSEON;  // enable HSE
  while ((RCC_CR & RCC_CR_HSERDY) == 0);     // wait for HSERDY
  
  // STEP 2: Set up clock source
  // ref RM0008 p92
  /*
  RCC_CFGR =  0 << 24 \   // MCO = none
            + 0 << 22 \   // USBPRE = 0
            + 2 << 18 \   // PLLMUL = 2 (16MHz)
            + 1 << 16 \   // PLLSRC = 1 (HSE = PLL in)
            + 0 << 14 \   // ADCPRE = 0
            + 0 << 11 \   // PPRE2 = 0 (APB2 = HCLK)
            + 0 << 8  \   // PPRE1 = 0 (APB1 = HCLK)
            + 0 << 4  \   // HPRE = 0 (AHB = SYSCLK)
            + 0;          // SYSCLK = HSI
  */
  RCC_CFGR =  0 << 24 \
            + 0 << 22 \
            + 2 << 18 \
            + 1 << 16 \
            + 0 << 14 \
            + 0 << 11 \
            + 0 << 8  \
            + 0 << 4  \
            + 0;

  // STEP 3: Enable PLL, wait for settling
  
  RCC_CR |= RCC_CR_PLLON;
  while ((RCC_CR & RCC_CR_PLLRDY) == 0);        // wait for PLLRDY

  // STEP 4: Set SYSCLK = PLL
  RCC_CFGR |= 0x00000002;
  
//	rcc_clock_setup_in_hse_8mhz_out_24mhz();
  
	//rcc_periph_clock_enable(RCC_GPIOC);
	RCC_APB2ENR |= 1 << 4;
	//rcc_periph_clock_enable(RCC_GPIOB);
	RCC_APB2ENR |= 1 << 3;

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


