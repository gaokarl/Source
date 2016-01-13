/*******************************************************************************
* Copyright (C) 2013-2015, Cypress Semiconductor Corporation. All rights reserved.
*
* This software, including source code, documentation and related
* materials ( "Software" ), is owned by Cypress Semiconductor
* Corporation ( "Cypress" ) and is protected by and subject to worldwide
* patent protection (United States and foreign), United States copyright
* laws and international treaty provisions. Therefore, you may use this
* Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software ( "EULA" ).
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the
* Software source code solely for use in connection with Cypress's
* integrated circuit products. Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO
* WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,
* BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE. Cypress reserves the right to make
* changes to the Software without notice. Cypress does not assume any
* liability arising out of the application or use of the Software or any
* product or circuit described in the Software. Cypress does not
* authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ( "High Risk Product" ). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*/
/******************************************************************************/

#include "mcu.h"

/** \file system_s6e1c3.c
 **
 ** FM0+ system initialization functions
 ** All adjustments can be done in belonging header file.
 **
 ** History:
 ** 2013-12-09  1.0  Edison Zhang  first version for FM0+ s6e1c3 series
 ** 2014-08-26  1.1  EH            Added to set LCR_PRSLD register
 ******************************************************************************/

/**
 ******************************************************************************
 ** System Clock Frequency (Core Clock) Variable according CMSIS
 ******************************************************************************/
uint32_t SystemCoreClock = __HCLK;

/**
 ******************************************************************************
 ** \brief  Update the System Core Clock with current core Clock retrieved from
 ** cpu registers.
 ** \param  none
 ** \return none
 ******************************************************************************/
void SystemCoreClockUpdate (void) {
  uint32_t masterClk;
  uint32_t u32RegisterRead; // Workaround variable for MISRA C rule conformance

  switch ((FM0P_CRG->SCM_CTL >> 5U) & 0x07U) {
    case 0u:                                 /* internal High-speed Cr osc.    */
      masterClk = __CLKHC;
      break;

    case 1u:                                 /* external main osc.             */
      masterClk = __CLKMO;
      break;

    case 2u:                                 /* PLL clock                      */
  // Workaround for preventing MISRA C:1998 Rule 46 (MISRA C:2004 Rule 12.2)
  // violation:
  //   "Unordered accesses to a volatile location"
      u32RegisterRead = (__CLKMO  * (((uint32_t)(FM0P_CRG->PLL_CTL2) & 0x1Fu) + 1u));
      masterClk = (u32RegisterRead / ((((uint32_t)(FM0P_CRG->PLL_CTL1) >> 4ul) & 0x0Fu) + 1u));
      break;

    case 4u:                                 /* internal Low-speed CR osc.     */
      masterClk = __CLKLC;
      break;

    case 5u:                                 /* external Sub osc.              */
      masterClk = __CLKSO;
      break;

    default:
      masterClk = 0ul;
      break;
  }

  switch (FM0P_CRG->BSC_PSR & 0x07u) {
    case 0u:
      SystemCoreClock = masterClk;
      break;

    case 1u:
      SystemCoreClock = masterClk / 2u;
      break;

    case 2u:
      SystemCoreClock = masterClk / 3u;
      break;

    case 3u:
      SystemCoreClock = masterClk / 4u;
      break;

    case 4u:
      SystemCoreClock = masterClk / 6u;
      break;

    case 5u:
      SystemCoreClock = masterClk /8u;
      break;

    case 6u:
      SystemCoreClock = masterClk /16u;
      break;

    default:
      SystemCoreClock = 0ul;
      break;
  }

}

/**
 ******************************************************************************
 ** \brief  Setup the microcontroller system. Initialize the System and update
 ** the SystemCoreClock variable.
 **
 ** \param  none
 ** \return none
 ******************************************************************************/
void SystemInit (void) {

#if (CLOCK_SETUP == CLOCK_SETTING_CMSIS)
  static uint8_t u8IoRegisterRead;  // Workaround variable for MISRA C rule conformance
#endif
  
#if (HWWD_DISABLE)                                 /* HW Watchdog Disable */
  FM0P_HWWDT->WDG_LCK = 0x1ACCE551u;                /* HW Watchdog Unlock */
  FM0P_HWWDT->WDG_LCK = 0xE5331AAEu;
  FM0P_HWWDT->WDG_CTL = 0u;                         /* HW Watchdog stop */
#endif
  
#if (CLOCK_SETUP == CLOCK_SETTING_CMSIS)                     /* Clock Setup */
  FM0P_CRG->BSC_PSR   = (uint8_t)BSC_PSR_Val;                 /* set System Clock presacaler */
  FM0P_CRG->APBC0_PSR = (uint8_t)APBC0_PSR_Val;               /* set APB0 presacaler */
  FM0P_CRG->APBC1_PSR = (uint8_t)APBC1_PSR_Val;               /* set APB1 presacaler */
  FM0P_CRG->SWC_PSR   = (uint8_t)(SWC_PSR_Val | (1ul << 7u)); /* set SW Watchdog presacaler */

  FM0P_CRG->CSW_TMR   = (uint8_t)CSW_TMR_Val;                 /* set oscillation stabilization wait time */
  
  if (SCM_CTL_Val & (1ul << 1u)) {                 /* Main clock oscillator enabled ? */
    FM0P_CRG->SCM_CTL |= (uint8_t)(1ul << 1u);            /* enable main oscillator */ 
    
    while (!((FM0P_CRG->SCM_STR) & (uint8_t)(1ul << 1u))) /* wait for Main clock oscillation stable */ 
    {}
  }
  
  if (SCM_CTL_Val & (1UL << 3))                     /* Sub clock oscillator enabled ? */
  {
    FM0P_CRG->SCM_CTL |= (1UL << 3);                // enable sub oscillator
    while (!(FM0P_CRG->SCM_STR & (1UL << 3)))       // wait for Sub clock oscillation stable
    {}
  }

  FM0P_CRG->PSW_TMR   =  (uint8_t)PSW_TMR_Val;                // set PLL stabilization wait time
  FM0P_CRG->PLL_CTL1  = (uint8_t) PLL_CTL1_Val;               // set PLLM and PLLK
  FM0P_CRG->PLL_CTL2  =  (uint8_t)PLL_CTL2_Val;               // set PLLN
  
  if (SCM_CTL_Val &  (uint8_t)(1ul << 4u)) {                 // PLL enabled? 
    FM0P_CRG->SCM_CTL  |=  (uint8_t)(1ul << 4u);              // enable PLL
    while (!(FM0P_CRG->SCM_STR &  (uint8_t)(1ul << 4u)))      // wait for PLL stable
    {}
  }

  if ((uint8_t)(SCM_CTL_Val &  0xE0u)==(uint8_t)0x80ul) {    /* Low-speed CR oscillation selected ? */
    u8IoRegisterRead = (FM0P_LSCRP->LCR_PRSLD & 0x3Fu);         /* Read LCR_PRSLD value */ 
    if(u8IoRegisterRead > (uint8_t)0ul) {
      uint32_t i;
      uint32_t u32WaitValue;
      u32WaitValue = (__CLKHC/__CLKLC)*u8IoRegisterRead;     /* caluculate wait value */
      FM0P_LSCRP->LCR_PRSLD = (uint8_t)0x00u;                   /* Set "0" to LCR_PRSLD register */
      for(i=0; i<u32WaitValue; i++);                         /* wait for changing clock */
    }
    FM0P_LSCRP->LCR_PRSLD = (uint8_t)LCR_PRSLD_Val;             /* set the division ratio of low-speed CR */
  }

  FM0P_CRG->SCM_CTL  |=  (uint8_t)(SCM_CTL_Val & 0xE0u);       // Set Master Clock switch
  
  // Workaround for preventing MISRA C:1998 Rule 46 (MISRA C:2004 Rule 12.2)
  // violations:
  //   "Unordered reads and writes to or from same location" and
  //   "Unordered accesses to a volatile location"
  do                                              
  {                                               
    u8IoRegisterRead = (FM0P_CRG->SCM_CTL & 0xE0u); 
  }while ((FM0P_CRG->SCM_STR & 0xE0u) != u8IoRegisterRead);
  
  if ((SCM_CTL_Val & 1u) != 1u) {             /* High-speed CR oscillator disabled ? */
    FM0P_CRG->SCM_CTL &= (uint8_t)(~1ul);      /* disable High-speed CR oscillator */ 
  }
  
#elif (CLOCK_SETUP == CLOCK_SETTING_NONE)
  
  // user defined clock setting
  
#else
  #error Clock setup type unknown!
#endif
  
#if (CR_TRIM_SETUP)
  /* CR Trimming Data  */
  if( 0x000003FFu != (FM0P_FLASH_IF->CRTRMM & 0x000003FFu) )
  {
    /* UnLock (MCR_FTRM) */
    FM0P_CRTRIM->MCR_RLR =  (uint32_t)0x1ACCE554u;
    /* Set MCR_FTRM */
    FM0P_CRTRIM->MCR_FTRM = (uint16_t)FM0P_FLASH_IF->CRTRMM;
    /* Set MCR_TTRM */
    FM0P_CRTRIM->MCR_TTRM = (uint16_t)(FM0P_FLASH_IF->CRTRMM >> 16u);
    /* Lock (MCR_FTRM) */
    FM0P_CRTRIM->MCR_RLR =  (uint32_t)0x00000000u;
  }
#endif // (CR_TRIM_SETUP)
}



