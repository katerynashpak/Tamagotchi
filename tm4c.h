
#ifndef __TM4C_H__
#define __TM4C_H__

#include <stdint.h>

typedef struct {                    // GPIO Register Map,
  volatile uint32_t DATA_Bits[255]; // 1 KB alias window

  volatile uint32_t DATA;     // 0x3FC
  volatile uint32_t DIR;      // 0x400
  volatile uint32_t IS;       // 0x404
  volatile uint32_t IBE;      // 0x408
  volatile uint32_t IEV;      // 0x40C
  volatile uint32_t IM;       // 0x410
  volatile uint32_t RIS;      // 0x414
  volatile uint32_t MIS;      // 0x418
  volatile uint32_t ICR;      // 0x41C
  volatile uint32_t AFSEL;    // 0x420

  volatile uint32_t RESERVED[55];

  volatile uint32_t DR2R;     // 0x500
  volatile uint32_t DR4R;     // 0x504
  volatile uint32_t DR8R;     // 0x508
  volatile uint32_t ODR;      // 0x50C
  volatile uint32_t PUR;      // 0x510
  volatile uint32_t PDR;      // 0x514
  volatile uint32_t SLR;      // 0x518
  volatile uint32_t DEN;      // 0x51C
  volatile uint32_t LOCK;     // 0x520
  volatile uint32_t CR;       // 0x524
  volatile uint32_t AMSEL;    // 0x528
  volatile uint32_t PCTL;     // 0x52C
  //...

} GPIO_Type;

typedef struct{
  //I2C Master
  volatile uint32_t MSA;      // 0x000
  volatile uint32_t MCS;      // 0x004
  volatile uint32_t MDR;      // 0x008
  volatile uint32_t MTPR;     // 0x00C
  volatile uint32_t MIMR;     // 0x010
  volatile uint32_t MRIS;     // 0x014
  volatile uint32_t MMIS;     // 0x018
  volatile uint32_t MICR;     // 0x01C
  volatile uint32_t MCR;      // 0x020
  volatile uint32_t MCLKOCNT; // 0x024
  volatile uint32_t MBMON;    // 0x02C
  volatile uint32_t MCR2;     // 0x038
  //...
} I2C_Type;

typedef struct {
  volatile uint32_t CTRL;
  volatile uint32_t LOAD;
  volatile uint32_t VAL;
  volatile uint32_t CALIB;
} SysTick_Type;

#define RCGCGPIO (*((volatile uint32_t *)0x400FE608))   // clock gating control
#define GPIOHBCTL (*((volatile uint32_t *)0x400FE06C))  // high performance bus control
#define GPIOA_AHB ((GPIO_Type *)0x40058000)             //PA ports
#define SysTick ((SysTick_Type *)0xE000E010)

#define RCGCI2C (*((volatile uint32_t *)0x400FE620))   // clock gating control
#define GPIOB_AHB ((GPIO_Type *)0x40059000)             //PB ports
#define I2C0 ((I2C_Type *)0x40020000)                   //I2C Master Slave Address (I2CMSA)


#endif // __TM4C_H__