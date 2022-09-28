#include <xc.h>

#include "project_settings.h"
#include "i2c.h"
#include "max6958.h"

#define MAX6958_I2C_ADDRESS             0b01110000
#define MAX6958_CMD_DECODE              0b00000001
#define MAX6958_CMD_INTENSITY           0b00000010
#define MAX6958_CMD_CONFIGURATION       0b00000100
#define MAX6958_CMD_TEST                0b00000111
#define MAX6958_CMD_DIGIT0              0b00100000
#define MAX6958_CMD_DIGIT1              0b00100001
#define MAX6958_CMD_DIGIT2              0b00100010
#define MAX6958_CMD_DIGIT3              0b00100011

#define MAX6958_NO_DECODE_ALL_DIGITS	0x0
#define MAX6958_HEX_DECODE_ALL_DIGITS	0xf
#define MAX6958_CLR_AND_NORMAL_OP       0x21

void MAX6958ClearAndNormalOperation(void) {
    // No decode will cause blank display on clear, rather than 0000
    MAX6958SendCommand(MAX6958_CMD_DECODE, MAX6958_NO_DECODE_ALL_DIGITS);
    MAX6958SendCommand(MAX6958_CMD_CONFIGURATION, MAX6958_CLR_AND_NORMAL_OP);
}

void MAX6958DisplayNumber(uint16_t num) {
    uint8_t dig;

    MAX6958SendCommand(MAX6958_CMD_DECODE, MAX6958_HEX_DECODE_ALL_DIGITS);
    dig = num / 1000;
    MAX6958SendCommand(MAX6958_CMD_DIGIT0, dig);
    dig = (num / 100) % 10;
    MAX6958SendCommand(MAX6958_CMD_DIGIT1, dig);
    dig = (num / 10) % 10;
    MAX6958SendCommand(MAX6958_CMD_DIGIT2, dig);
    dig = num % 10;
    MAX6958SendCommand(MAX6958_CMD_DIGIT3, dig);
}

void MAX6958Init(void) {
    MAX6958ClearAndNormalOperation();
    MAX6958SendCommand(MAX6958_CMD_INTENSITY, 40);
}

void MAX6958SendCommand(uint8_t cmd, uint8_t data) {
    I2CStart();
    I2CSend(MAX6958_I2C_ADDRESS);
    I2CSend(cmd);
    I2CSend(data);
    I2CStop();
    __delay_us(30);
}

void MAX6958BlinkNumber(uint16_t number, uint8_t repetitions, uint16_t onMs, uint16_t offMs) {
    uint8_t i;
    uint16_t j;

    for (i = 0; i < repetitions; i++) {
        MAX6958ClearAndNormalOperation();
        // Have to loop for the delays since __delay_ms is a macro that does not accept a
        // variable as a an argument
        for (j = 0; j < offMs; j++) {
            __delay_ms(1);
        }
        
        MAX6958DisplayNumber(number);
        for (j = 0; j < onMs; j++) {
            __delay_ms(1);
        }
    }
}
