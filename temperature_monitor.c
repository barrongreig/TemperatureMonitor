/*
 * File:   temperature_monitor.c
 * Author: Barron Greig
 *
 * Created on July 28, 2021, 5:02 PM
 */

// PIC16F15313 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (EC above 8MHz; PFM set to high power)
#pragma config RSTOSC = HFINT1     // Power-up default value for COSC bits (EXTOSC operating per FEXTOSC bits)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = OFF       // Fail-Safe Clock Monitor Enable bit (FSCM timer enabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config LPBOREN = OFF    // Low-Power BOR enable bit (ULPBOR disabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) set to 1.9V on LF, and 2.45V on F Devices)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = ON     // Peripheral Pin Select one-way control (The PPSLOCK bit can be cleared and set only once in software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep; SWDTEN ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Register not write protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF not write protected)
#pragma config LVP = ON         // Low Voltage Programming Enable bit (Low Voltage programming enabled. MCLR/Vpp pin function is MCLR.)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <pic16f15313.h>



#include "one_wire.h"
#include "ds18b20.h"
#include "i2c.h"
#include "max6958.h"


#define DLY_MODE_CHANGE             300
#define DLY_TEMPERATURE_DISPLAY     500

/* FUTURE USE - if EEPROM becomes available in 8 pin form factor
#define MSG_SENSOR_COUNT_UPDATED    9800
 */

#define MSG_RESOLUTION_UPDATED      9900
#define MSG_SENSOR_READ_ERR         9901
        

/* FUTURE USE - if EEPROM becomes available in 8 pin form factor
__eeprom uint8_t savedSensorCount = 0;
*/


void MicrocontrollerInit(void) {
    // Watchdog timer - I switched this off in config tho
    //WDTCON0bits.WDTPS = 0b01010; // 0b01010 -> 1:32768, ~1s PIC16f15313

    // WDTCONbits.WDTPS=0b01010; // 0b01010 -> 1:32768, ~1s PIC16f18313
    // Set up pin multiplexer
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCK = 0x00; // unlock PPS
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCK = 0x01; // lock PPS

    ANSELA = 0;
    INTCONbits.GIE = 0;
    TRISAbits.TRISA0 = 1;
    // Configure unused RA1 as output so that it is not left floating.
    TRISAbits.TRISA1 = 0;
    ANSELAbits.ANSA0 = 0;
    WPUAbits.WPUA0 = 1;
    I2CInit();
}


void main(void) {

    MicrocontrollerInit();
    MAX6958Init();

    uint8_t sensorCount;
    bool isResolutionUpdated;
    
    DS18B20_VerifySensorConfiguration((uint8_t) 0, &sensorCount, &isResolutionUpdated);
    
    if (isResolutionUpdated) {
        MAX6958BlinkNumber(MSG_RESOLUTION_UPDATED, (uint8_t) 5, 1000, 200);        
    }
    
    /* FUTURE USE - if EEPROM becomes available in 8 pin form factor
    if (sensorCount != savedSensorCount) {
        savedSensorCount = sensorCount;
        MAX6958BlinkNumber(MSG_SENSOR_COUNT_UPDATED + savedSensorCount, (uint8_t) 5, 1000, 200);
    }*/
    
    bool isMaxMode = false;    
    while (true) {
        bool isReadOK = OWFirst();
        if (!isReadOK) {
            MAX6958BlinkNumber(MSG_SENSOR_READ_ERR, (uint8_t) 5, 1000, 200);
            continue;
        }

        uint8_t maxTemperatureSensorNum = 0;
        uint8_t maxTemperature = 0;
        sensorCount = 0;
        while (isReadOK) {
            uint8_t temperature = (uint8_t) DS18B20_GetTempClamped(0, 99);
            if (!PORTAbits.RA0) {
                isMaxMode = !isMaxMode;
                maxTemperatureSensorNum = 0;
                maxTemperature = 0;
                MAX6958ClearAndNormalOperation();
                __delay_ms(DLY_MODE_CHANGE);
            }

            if (temperature > maxTemperature) {
                maxTemperatureSensorNum = sensorCount;
                maxTemperature = temperature;
            }
            if (!isMaxMode) {
                MAX6958DisplayNumber(sensorCount * 100 + temperature);
                __delay_ms(DLY_TEMPERATURE_DISPLAY);
            }
            sensorCount++;

            isReadOK = OWNext();
        }
        
        if (isMaxMode) {
            MAX6958DisplayNumber(maxTemperatureSensorNum * 100 + maxTemperature);
        }
    }
}
