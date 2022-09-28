/* 
 * File:   max6958.h
 * Author: Bazazzz
 *
 * Created on August 29, 2021, 1:47 PM
 */

#ifndef MAX6958_H
#define	MAX6958_H

#include <xc.h>

void MAX6958BlinkNumber(uint16_t number, uint8_t repetitions, uint16_t onMs, uint16_t offMs);
void MAX6958ClearAndNormalOperation(void);
void MAX6958DisplayNumber(uint16_t num);
void MAX6958Init(void);
void MAX6958SendCommand(uint8_t cmd, uint8_t data);

#endif	/* MAX6958_H */

