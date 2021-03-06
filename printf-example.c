#ifndef __NO_SYSTEM_INIT
void SystemInit()
{}
#endif

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
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
  
  // STEP 1 - Enable IO clock for IOPB, IOPB
  // Ref: RM0008 p103
	RCC_APB2ENR |= (1 << 4) + (1 << 3);

  // STEP 2 - Configure GPIOB12-15 as 2 MHz output
  // Ref: RM0008 p156
  GPIOB_CRH=(GPIOB_CRH & 0x0000FFFF) + 0x22220000;

  // STEP 3 - Configure GPIOC6-8 as inputs
  
  GPIOC_CRL=(GPIOC_CRL & 0x00FFFFFF) + 0x88000000;
  GPIOC_CRH=(GPIOC_CRH & 0xFFFFFFF0) + 0x00000008;  
  
//	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
//		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO12|GPIO13|GPIO14|GPIO15);

//	gpio_set_mode(GPIOC, GPIO_MODE_INPUT,
//		      GPIO_CNF_INPUT_PULL_UPDOWN, GPIO6|GPIO7|GPIO8);
	
	// STEP 4 - Set GPIOC6-8
	GPIOC_BSRR = (1 << 6)+ (1 << 7) + (1 << 8);	      
//  gpio_set(GPIOC,GPIO6|GPIO7|GPIO8);
  
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
            + 0 << 14 \   // ADCPRE = 0               Max: 14MHz
            + 0 << 11 \   // PPRE2 = 0 (APB2 = HCLK) Max: 72MHz
            + 0 << 8  \   // PPRE1 = 0 (APB1 = HCLK) Max: 36MHz
            + 0 << 4  \   // HPRE = 0 (AHB = SYSCLK) Max: 72MHz
            + 0;          // SYSCLK = HSI
  */
   RCC_CFGR =   (0 << 24) \
              + (0 << 22) \
              + (3 << 18) \
              + (1 << 16) \
              + (0 << 14) \
              + (0 << 11) \
              + (0 << 8) \
              + (0 << 4) \
              + 0;
    //RCC_CFGR = 0x000D0000;

  // STEP 3: Set Flash wait states (SYSCLK <=24MHz:0 , 24-48MHz:1, 48-72MHz:2)
  // Ref. RM0008 p54
  FLASH_ACR=(FLASH_ACR & 0xFFFFFFF8)|0;
  
  // STEP 4: Enable PLL, wait for settling
  
  RCC_CR |= RCC_CR_PLLON;
  while ((RCC_CR & RCC_CR_PLLRDY) == 0);        // wait for PLLRDY

  // STEP 5: Set SYSCLK = PLL
  RCC_CFGR |= 0x00000002;
    
}

void timer_setup(void) {

  // STEP 1: Set systick clock source
  //systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
  STK_CSR &= ~(1 << 2);    // clock source = AHB/8
  
  // STEP 2: Set autoreload counter
  //systick_set_reload(1499);
  STK_RVR=1499;
  
  // STEP 3: Enable systick interrupt
  //systick_interrupt_enable();
  STK_CSR |= (1 << 1);
  
  // STEP 4: GO
  //systick_counter_enable();
  STK_CSR |= (1 << 0);
  
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


