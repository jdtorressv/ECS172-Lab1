//*****************************************************************************
// Authors              - Shivam Desai, Joseph Torres
// Application Name     - Lab1
// Application Overview - The purpose of this lab was to investigate the use of
//                        the software development tools that we can expect to
//                        encounter frequently over the rest of the quarter,
//                        including Code Composer Studio IDE and SDK, the Texas
//                        Instruments Pin Mux Tool, CCS Uniflash, and Putty. The
//                        resulting program built using these tools samples some
//                        of the TI CC3200 Launchpad’s core functionalities,
//                        including interfacing to a terminal emulator over a UART
//                        serial connection, LED control in response to switch
//                        input, sending a voltage signal to P18 on the P2 header,
//                        and loading .bin files into the board’s flash memory.
//
//*****************************************************************************

// Standard includes
#include <stdio.h>

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "uart_if.h"
#include "uart.h"

// Common interface includes
#include "gpio_if.h"

#include "pin_mux_config.h"

#define APPLICATION_VERSION     "1.1.1"
#define MAX_COUNT 7
#define NUM_COLORS 3
#define APP_NAME             "GPIO"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
//void LEDBlinkyRoutine();
static void DisplayBanner(char* AppName);
static void BoardInit(void);
void checkInput();
void blinkAll();
void countUp();
//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//*****************************************************************************

//*****************************************************************************
//
//! Configures the pins as GPIOs and peroidically toggles the lines
//!
//! \param None
//!
//! This function
//!    1. Configures 3 lines connected to LEDs as GPIO
//!    2. Sets up the GPIO pins as output
//!    3. Periodically toggles each LED one by one by toggling the GPIO line
//!
//! \return None
//
//*****************************************************************************

static void DisplayBanner(char * AppName)
{

    Report("\n\n\n\r");
    Report("\t\t *************************************************\n\r");
    Report("\t\t     CC3200 %s Application       \n\r", AppName);
    Report("\t\t *************************************************\n\r");
    Report("\n\n\n\r");
}
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************

static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

void checkInput(){
  long SW3 = GPIOPinRead(GPIOA1_BASE, 0x20);
  long SW2 = GPIOPinRead(GPIOA2_BASE, 0x40);

  if (SW2 == 0x40){
    GPIOPinWrite(GPIOA3_BASE, 0x10, 0x10);
    blinkAll();
  } else if (SW3 == 0x20){
    GPIOPinWrite(GPIOA3_BASE, 0x10, 0x0);
    countUp();
  }
}

void blinkAll(){
  Message("\t\t\t      SW2 Pressed     \n\r");
  GPIO_IF_LedOff(MCU_ALL_LED_IND);
  while (GPIOPinRead(GPIOA1_BASE, 0x20) != 0x20){
      GPIO_IF_LedOn(MCU_ALL_LED_IND);
      MAP_UtilsDelay(8000000);
      if (GPIOPinRead(GPIOA1_BASE, 0x20) == 0x20) {
          GPIOPinWrite(GPIOA3_BASE, 0x10, 0x0);
          countUp();
      }
      GPIO_IF_LedOff(MCU_ALL_LED_IND);
      MAP_UtilsDelay(8000000);
  }
  GPIOPinWrite(GPIOA3_BASE, 0x10, 0x0);
  countUp();
}

void countUp() {
  Message("\t\t\t      SW3 Pressed     \n\r");
  GPIO_IF_LedOff(MCU_ALL_LED_IND);
  int count = 0;
  while(GPIOPinRead(GPIOA2_BASE, 0x40) != 0x40) {
      int stripper = count;
      int color = 0;
      while (color < NUM_COLORS) {
          if (stripper % 2 == 1) {
              switch(color) {
              case 0: GPIO_IF_LedOn(MCU_RED_LED_GPIO);
              break;
              case 1: GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
              break;
              case 2: GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
              break;
              }
          }
          else {
              switch(color) {
              case 0: GPIO_IF_LedOff(MCU_RED_LED_GPIO);
              break;
              case 1: GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
              break;
              case 2: GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
              break;
              }
          }
          stripper /= 2;
          color++;
      }
      count++;
      if (count > MAX_COUNT)
          count = 0;
      MAP_UtilsDelay(8000000);
      //checkInput();
  }
  GPIOPinWrite(GPIOA3_BASE, 0x10, 0x10);
  blinkAll();
}

void main()
{
    BoardInit();
    PinMuxConfig();
    GPIO_IF_LedConfigure(LED1|LED2|LED3);
    InitTerm();
    ClearTerm();
    DisplayBanner(APP_NAME);
    Message("\t\t****************************************************\n\r");
    Message("\t\t    Push SW3 to start LED binary counting     \n\r");
    Message("\t\t    Push SW2 to blink LEDs on and off     \n\r");
    Message("\t\t****************************************************\n\r");
    Message("\n\n\n\r");
    while (1){
        checkInput();
    }
}
