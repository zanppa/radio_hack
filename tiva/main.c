#define PART_TM4C123GH6PM
typedef unsigned char bool;

#include <stdint.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_i2c.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"

#include "driverlib/uart.h"
#include "utils/uartstdio.h"
//#include "peripherials.h"

#define I2C1_SLAVE_BASE I2C1_BASE

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

/* variables, const */

#define SLAVE_OWN_ADDRESS   0x50

static unsigned char buffer[256] = {
        0x00, 0x01, 0x00, 0x01, 0x04, 0x1b, 0x24, 0xb0, // <-- 4 ja 5 FM band number
        0x3c, 0x27, 0x70, 0x10, 0x30, 0x1e, 0x62, 0x8d, 
        0x00, 0xaf, 0xbc, 0x01, 0x00, 0x22, 0x1c, 0x00,
        0x64, 0x40, 0x00, 0x00, 0x00, 0x79, 0x00, 0x00, 
        0x08, 0xd6, 0x01, 0xc7, 0x00, 0x00, 0x1b, 0xd5, 

        0x00, 0x01, 0x02, 0x9c, 0x64, 0xa1, 0x80, 0x00, // <-- 4 vikaa xtal low bits ja high bits (modified)
//        0x00, 0x01, 0x02, 0x9c, 0x80, 0x00, 0x80, 0x00, // <-- 4 vikaa xtal low bits ja high bits (~original)

        0x14, 0x01, 0x11, 0x00, 0x80, 0x1f, 0x04, 0x00,
        0x08, 0x05, 0x40, 0x00, 0x0e, 0x0b, 0x70, 0x00, 
        0x00, 0x32, 0x07, 0x42, 0x21, 0x20, 0x31, 0x00, 
        0x04, 0x9f, 0x01, 0x1b, 0x28, 0x8d, 0x01, 0x00, 
        0x1c, 0x70, 0x34, 0x40, 0x10, 0x00, 0x30, 0xea, 
        0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 
        0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 
        0x00, 0x02, 0x02, 0x02, 0x03, 0x03, 0x08, 0xa0, 

        0xc1, 0x1a, 0x05, 0x5d, 0x01, 0x4D, 0x08, 0x20,  // <-- 2 ekaa BANDCFG, 2 vikaa FM2 low edge (0x76-0x77) (modified)
//        0xc1, 0x1a, 0x06, 0xd2, 0x01, 0xA8, 0x08, 0x20,  // <-- 2 ekaa BANDCFG, 2 vikaa FM2 low edge (0x76-0x77) (~original)

        0x00, 0x5a, 0x05, 0x00, 0x00, 0xeb, 0x02, 0x01,  // <-- 2 ekaa FM2 chan num  (0x78-0x79)
        0x00, 0x7c, 0x17, 0x0c, 0x17, 0x44, 0x57, 0x60, 
        0x76, 0xdc, 0x37, 0x7c, 0x56, 0xf2, 0x37, 0xb2, 
        0x38, 0x1e, 0x02, 0x5b, 0x02, 0x76, 0x02, 0xeb, 
        0x03, 0x21, 0x03, 0x60, 0x03, 0xd5, 0x04, 0x14, 
        0x04, 0xc8, 0x33, 0x33, 0x33, 0x33, 0xa0, 0x04, 
        0x00, 0x00, 0x03, 0xec, 0x00, 0x00, 0x00, 0x11, 
        0x72, 0x10, 0x3c, 0x32, 0x00, 0x37, 0x24, 0x1e, 
        0xad, 0xdc, 0x00, 0x00, 0x81, 0x00, 0xf3, 0x00, 
        0x42, 0x80, 0xd5, 0x00, 0x15, 0x00, 0x02, 0x7f, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

volatile unsigned char addr = 0x00;

typedef volatile unsigned short vus;
static vus receivedCC2541;
volatile unsigned char intflag = 0;

void I2C1SlaveIntHandler(void)
{
    uint32_t stat;

    stat = I2CSlaveStatus(I2C1_SLAVE_BASE);
    
    I2CSlaveIntClear(I2C1_SLAVE_BASE);
    I2CSlaveIntClearEx(I2C1_SLAVE_BASE,I2C_SLAVE_INT_DATA);

    //while(I2CSlaveStatus(I2C1_SLAVE_BASE) & I2C_SLAVE_ACT_NONE);
    //switch(I2CSlaveStatus(I2C1_SLAVE_BASE);)
    switch(stat)
    {
        case I2C_SLAVE_ACT_RREQ_FBR:
        {
            // here when Master sends exact 1 B - normal write
            addr = I2CSlaveDataGet(I2C1_SLAVE_BASE);
            break;
        }

        case I2C_SLAVE_ACT_RREQ:
        {
            // here when Master sends WRITE and more than 1 B to send - burst write
            receivedCC2541 = I2CSlaveDataGet(I2C1_SLAVE_BASE);
            break;
        }

        case I2C_SLAVE_ACT_TREQ:
        {
            // when transmit request
            I2CSlaveDataPut(I2C1_SLAVE_BASE, buffer[addr++]);
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,GPIO_PIN_3);
            if(addr & 0x40)
              GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);
            else
              GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
            break;
        }
        default:
            // none, why would that happen?
            break;
    }
    intflag = 1;
}

int main(void)
 {
    //FPUEnable();
    //FPULazyStackingEnable();
    
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    //SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL| SYSCTL_OSC_INT);
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
    I2CSlaveEnable(I2C1_SLAVE_BASE);
    I2CSlaveInit(I2C1_SLAVE_BASE, SLAVE_OWN_ADDRESS);

    /* Enable interrupts */
    I2CSlaveIntEnable(I2C1_SLAVE_BASE);
    I2CSlaveIntEnableEx(I2C1_SLAVE_BASE, I2C_SLAVE_INT_DATA);
    I2CIntRegister(I2C1_SLAVE_BASE, I2C1SlaveIntHandler);
    IntEnable(INT_I2C1);
    IntMasterEnable();

    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,GPIO_PIN_1);

    // If SW1 is pressed, change the FM2 band to be from
    // (actual) 120 Mhz to 138 MHz (= about 94.3 MHz to 108.5 MHz)
    if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4))
    {
      // FM2 low channel at dec 118, 119 set to 94.3 MHZ / 0.05 = 0x075E
      buffer[118] = 0x07;
      buffer[119] = 0x53;

      // FM2 chan number at dec 120, 121 set to 14.2 / 0.05 = 0x011C
      buffer[120] = 0x01;
      buffer[121] = 0x1C;

      GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);
    }

    // If SW2 is pressed, change the FM2 band to be from
    // (actual) 100 Mhz to 120 MHz (= about 78 MHz to 94.3 MHz)
    if(!GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))
    {
      // FM2 low channel at dec 118, 119 set to 78 
      buffer[118] = 0x06;
      buffer[119] = 0x18;

      // FM2 chan number at dec 120, 121 set to 16.3 / 0.05 = 0x0146
      buffer[120] = 0x01;
      buffer[121] = 0x46;

      GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);
    }



    while(1)
    {
        if(intflag)
        {
            intflag = 0;
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0);
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
            if(addr)
              GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0);
            else
              GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,GPIO_PIN_3);
        }
    }
    
    return 0;
 }
