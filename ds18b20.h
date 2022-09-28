#ifndef _DS18B20_H
#define _DS18B20_H

#include <stdbool.h>

#include "project_settings.h"
#include "one_wire.h"


uint8_t     DS18B20_GetCurrentResolutionBits();
int16_t     DS18B20_GetTemp16(void);
int8_t      DS18B20_GetTempClamped(int8_t minTemp, int8_t maxTemp);
int8_t      DS18B20_GetTempDegrees(void);
void        DS18B20_ReadScratchpad(void);
void        DS18B20_UpdateAllSensorResolutionBits(void);
void        DS18B20_VerifySensorConfiguration(uint8_t requestedResolutionBits, uint8_t *sensorCount,
                                             bool *updateResolution);


#endif /* _DS18B20_H */

