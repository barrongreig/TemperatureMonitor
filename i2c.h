/* 
 * File:   i2c.h
 * Author: Bazazzz
 *
 * Created on August 29, 2021, 1:31 PM
 */

#ifndef I2C_H
#define	I2C_H

void I2CAck(void);
void I2CInit(void);
void I2CNak(void);
void I2CRestart(void);
void I2CSend(uint8_t u8);
void I2CStart(void);
void I2CStop(void);
void I2CWait(void);

#endif	/* I2C_H */

