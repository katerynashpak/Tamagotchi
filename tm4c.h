/**************************************************************************//**
 * @file    tm4c.h
 * @brief    
 * @version  
 * @date     
 ******************************************************************************/

typedef struct { //GPIO Register Map, 
    volatile uint32_t DATA_Bits[255]; //1 KB alias window

    volatile uint32_t DATA;             //0x3FC
    volatile uint32_t DIR;              //0x400
    volatile uint32_t IS;               //0x404
    volatile uint32_t IBE;              //0x408
    volatile uint32_t IEV;              //0x40C
    volatile uint32_t IM;               //0x410
    volatile uint32_t RIS;              //0x414
    volatile uint32_t MIS;              //0x418
    volatile uint32_t ICR;              //0x41C
    volatile uint32_t AFSEL;            //0x420

    volatile uint32_t RESERVED[55];
    
    volatile uint32_t DR2R;             //0x500
    volatile uint32_t DR4R;             //0x504
    volatile uint32_t DR8R;             //0x508
    volatile uint32_t ODR;              //0x50C
    volatile uint32_t PUR;              //0x510
    volatile uint32_t PDR;              //0x514
    volatile uint32_t SLR;              //0x518
    volatile uint32_t DEN;              //0x51C
    //...

} GPIO_Type;


#define RCGCGPIO (*((volatile uint32_t *)0x400FE608)) //clock gating control
#define GPIOHBCTL (*((volatile uint32_t *) 0x400FE06C))// high performance bus control


#define GPIOA_AHB ((GPIO_Type *) 0x40058000)



