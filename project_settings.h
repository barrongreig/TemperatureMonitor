#ifndef PROJECT_SETTINGS_H
#define	PROJECT_SETTINGS_H

#include <xc.h>   
#include <stdbool.h>
#include <pic16f15313.h>

#define _XTAL_FREQ      1000000.0

#define OWOutputLow     TRISAbits.TRISA5 = 0
#define OWOutputHigh    TRISAbits.TRISA5 = 1

#define I2C_SCL         4
#define I2C_SDA         2
#define I2C_PPS_SCL     RA4PPS = 0x15
#define I2C_PPS_SDA     RA2PPS = 0x16
#define I2C_TRIS_SCL    TRISAbits.TRISA4
#define I2C_TRIS_SDA    TRISAbits.TRISA2
#define I2C_BAUD_RATE   100000
#define I2C_BRG         ((_XTAL_FREQ/4)/I2C_BAUD_RATE-1)


#endif	/* PROJECT_SETTINGS_H */

