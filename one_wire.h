#ifndef ONE_WIRE_H
#define	ONE_WIRE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>

#define OW_CMD_MATCHROM    0x55
#define OW_CMD_SKIPROM     0xCC


void        OWAddrDevice(uint8_t nAddrMethod);
bool        OWFirst(void);
bool        OWNext(void);
uint8_t     OWReadByte(void);
bool        OWReset(void);
bool        OWSearch(void);
void        OWWriteByte(uint8_t val_u8);

#endif	/* ONE_WIRE_H */

