
#include <xc.h>
#include <pic16f15313.h>

#include "project_settings.h"


#
void I2CInit(void) {
    I2C_TRIS_SCL = 1;
    I2C_TRIS_SDA = 1;
    SSP1STATbits.SMP = 1; /* Slew rate disabled */
    SSP1CON1bits.SSPM = 8; // = 0x28; /* SSPEN = 1, I2C Master mode, clock = _XTAL_FREQ/(4 * (SSPADD + 1)) */
    SSP1CON1bits.SSPEN = 1;
    I2C_PPS_SCL; 
    I2C_PPS_SDA;
    SSP1CLKPPSbits.SSP1CLKPPS = I2C_SCL;
    SSP1DATPPSbits.SSP1DATPPS = I2C_SDA;
    SSP1ADD = (uint8_t) I2C_BRG;
}

void I2CStart(void) {
    SSP1CON2bits.SEN = 1; /* Start condition enabled */
    while (SSP1CON2bits.SEN) /* automatically cleared by hardware */ { /* wait for start condition to finish */
        NOP();
    }
}

void I2CStop(void) {
    SSP1CON2bits.PEN = 1; /* Stop condition enabled */
    while (SSP1CON2bits.PEN) /* Wait for stop condition to finish */ { /* PEN automatically cleared by hardware */
        NOP();
    }
}

void I2CRestart(void) {
    SSP1CON2bits.RSEN = 1; /* Repeated start enabled */
    while (SSP1CON2bits.RSEN) /* wait for condition to finish */ {
        NOP();
    }
}

void I2CAck(void) {
    SSP1CON2bits.ACKDT = 0; /* Acknowledge data bit, 0 = ACK */
    SSP1CON2bits.ACKEN = 1; /* Ack data enabled */
    while (SSP1CON2bits.ACKEN) /* wait for ack data to send on bus */ {
        NOP();
    }
}

void I2CNak(void) {
    SSP1CON2bits.ACKDT = 1; /* Acknowledge data bit, 1 = NAK */
    SSP1CON2bits.ACKEN = 1; /* Ack data enabled */
    while (SSP1CON2bits.ACKEN) /* wait for ack data to send on bus */ {
        NOP();
    }
}

void I2CWait(void) {
    while ((SSP1CON2 & 0x1F) || (SSP1STATbits.R_nW /* .R_NOT_W*/)) /* wait for any pending transfer */ {
        NOP();
    }
}

void I2CSend(uint8_t u8) {
    SSP1BUF = u8; /* Move data to SSPBUF */
    while (SSP1STATbits.BF) /* wait till complete data is sent from buffer */ {
        NOP();
    }
    I2CWait(); /* wait for any pending transfer */
}
