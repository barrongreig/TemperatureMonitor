#include <stdbool.h>

#include "project_settings.h"
#include "one_wire.h"
#include "ds18b20.h"

/* -------------------------------------------------------------------------- */
/*                         DS1820 Timing Parameters                           */
/* -------------------------------------------------------------------------- */

#define DS18B20_MSTR_BITSTART   3    
#define DS18B20_COPYSCRPAD_DLY  10
// 9-bit temperature conversion time
#define DS18B20_TEMPCONVERT_DLY 94 



/* -------------------------------------------------------------------------- */
/*                            DS18B20 Registers                                */
/* -------------------------------------------------------------------------- */

#define DS18B20_REG_TEMPLSB          0
#define DS18B20_REG_TEMPMSB          1
#define DS18B20_REG_CONFIGURATION    4
#define DS18B20_REG_CNTREMAIN        6
#define DS18B20_REG_CNTPERSEC        7
#define DS18B20_SCRPADMEM_LEN        9    



/* -------------------------------------------------------------------------- */
/*                            DS18B20 Commands                                 */
/* -------------------------------------------------------------------------- */

#define DS18B20_CMD_SEARCHROM     0xF0
#define DS18B20_CMD_READROM       0x33
#define DS18B20_CMD_CONVERTTEMP   0x44
#define DS18B20_CMD_COPYSCRPAD    0x48
#define DS18B20_CMD_WRITESCRPAD   0x4E
#define DS18B20_CMD_RECALLEE      0xB8
#define DS18B20_CMD_READSCRPAD    0xBE



/* -------------------------------------------------------------------------- */
/*                            static variables                                */
/* -------------------------------------------------------------------------- */

static __bit bDoneFlag;
static uint8_t nLastDiscrepancy_u8;
static uint8_t scrpad[DS18B20_SCRPADMEM_LEN];
static uint8_t resolutionBits = 3;






/* -------------------------------------------------------------------------- */
/*                             API                                            */
/* -------------------------------------------------------------------------- */



void DS18B20_ReadScratchpad(void) {
    OWReset();
    OWAddrDevice(OW_CMD_MATCHROM);
    OWWriteByte(DS18B20_CMD_READSCRPAD);

    uint8_t i;
    for (i = 0; i < DS18B20_SCRPADMEM_LEN; i++) {
        scrpad[i] = OWReadByte();
    }
}

uint8_t DS18B20_GetCurrentResolutionBits() {
    return (scrpad[DS18B20_REG_CONFIGURATION] >> 5) & 0b11;
}

void DS18B20_UpdateAllSensorResolutionBits(void) {

    DS18B20_ReadScratchpad();
    scrpad[4] = ((uint8_t) (resolutionBits << 5)) | (uint8_t) 0b011111;
    OWReset();
    OWAddrDevice(OW_CMD_SKIPROM);
    OWWriteByte(DS18B20_CMD_WRITESCRPAD);
    OWWriteByte(scrpad[2]);
    OWWriteByte(scrpad[3]);
    OWWriteByte(scrpad[4]);
    OWReset();
    OWAddrDevice(OW_CMD_SKIPROM);
    OWWriteByte(DS18B20_CMD_COPYSCRPAD);
    __delay_us(DS18B20_COPYSCRPAD_DLY);
    OWReset();
}

int16_t DS18B20_GetTemp16(void) {
    int16_t temp_16;
    uint8_t i;
    
    OWReset();
    OWAddrDevice(OW_CMD_MATCHROM);
    OWOutputHigh;
    OWWriteByte(DS18B20_CMD_CONVERTTEMP);

    // Temperature conversion time is proportional to 2 to the power of the resolutionBits value
    // All DS18B20 sensors are assumed to have been configured to the same resolutionBits value
    for (i = 0; i < 2 << resolutionBits; i++) {
        __delay_ms(DS18B20_TEMPCONVERT_DLY);
    }

    DS18B20_ReadScratchpad();

    temp_16 = scrpad[DS18B20_REG_TEMPMSB] & 0b111;
    temp_16 = temp_16 << 8;
    temp_16 = temp_16 | scrpad[DS18B20_REG_TEMPLSB];

    if (scrpad[DS18B20_REG_TEMPMSB] & 0b10000000) temp_16 = -temp_16;
    return temp_16;
}

int8_t DS18B20_GetTempDegrees(void) {
    int16_t temp_16 = DS18B20_GetTemp16();
    temp_16 = temp_16 & 0b11111111111;

    temp_16 = temp_16 >> 4;
    if (scrpad[DS18B20_REG_TEMPMSB] & 0b10000000) temp_16 = -temp_16;

    return (int8_t) temp_16;
}

int8_t DS18B20_GetTempClamped(int8_t minTemp, int8_t maxTemp) {
    int8_t degrees = DS18B20_GetTempDegrees();
    if (degrees < minTemp) {
        degrees = minTemp;
    } else if (degrees > maxTemp) {
        degrees = maxTemp;
    }
    return degrees;
}

void DS18B20_VerifySensorConfiguration(uint8_t requestedResolutionBits, uint8_t *sensorCount,
        bool *isUpdatedResolution) {

    *isUpdatedResolution = false;
    *sensorCount = 0;
    resolutionBits = requestedResolutionBits;
    bool isReadOK = OWFirst();
    while (isReadOK) {
        DS18B20_ReadScratchpad();
        if (DS18B20_GetCurrentResolutionBits() != requestedResolutionBits) {
            *isUpdatedResolution = true;
        }
        (*sensorCount)++;
        isReadOK = OWNext();
    }

    if (*isUpdatedResolution) {
        DS18B20_UpdateAllSensorResolutionBits();
    }
}

