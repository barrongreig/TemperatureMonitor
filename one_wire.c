#include <stdbool.h>
#include "one_wire.h"
#include "project_settings.h"


#define OW_ADDR_LEN        8

#define OW_BITREAD_DLY     60    
#define OW_BITWRITE_DLY    60 
#define OW_PRESENCE_FIN    480
#define OW_PRESENCE_WAIT   70   
#define OW_RST_PULSE       480  


unsigned char ROM_NO[OW_ADDR_LEN];
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
bool LastDeviceFlag;
uint8_t crc8;



/*******************************************************************************
 * FUNCTION:   OW_Reset
 * PURPOSE:    Initializes the DS1820 device.
 *
 * INPUT:      -
 * OUTPUT:     -
 * RETURN:     TRUE if at least one device is on the 1-wire bus, FALSE otherwise
 ******************************************************************************/
bool OWReset(void)
{
   bool bPresPulse;

   OWOutputHigh;
   LATAbits.LATA5 = 0; // FIXME
   
   /* reset pulse */
   OWOutputLow;
   __delay_us(OW_RST_PULSE);
   OWOutputHigh;

   /* wait until pullup pulls 1-wire bus to high */
   __delay_us(OW_PRESENCE_WAIT);

   /* get presence pulse */
   
   TRISAbits.TRISA5 = 1;
   bPresPulse = PORTAbits.RA5;

   __delay_us(OW_PRESENCE_FIN);
   
   // Need to invert the buss state to return TRUE for 1 or more
   // connected devices
   return !bPresPulse;
}


/*******************************************************************************
 * FUNCTION:   DS1820_ReadBit
 * PURPOSE:    Reads a single bit from the DS1820 device.
 *
 * INPUT:      -
 * OUTPUT:     -
 * RETURN:     bool        value of the bit which as been read form the DS1820
 ******************************************************************************/
bool OWReadBit(void)
{
   bool bBit;

   OWOutputLow;
   __delay_us(2);
   OWOutputHigh;
   __delay_us(8);
   bBit = PORTAbits.RA5;

   __delay_us(OW_BITREAD_DLY);

   return (bBit);
}


/*******************************************************************************
 * FUNCTION:   DS1820_WriteBit
 * PURPOSE:    Writes a single bit to the DS1820 device.
 *
 * INPUT:      bBit        value of bit to be written
 * OUTPUT:     -
 * RETURN:     -
 ******************************************************************************/
void OWWriteBit(bool bBit)
{
   OWOutputLow;
   __delay_us(1);

   if (bBit)
   {
      OWOutputHigh;
   }
   __delay_us(OW_BITWRITE_DLY);
   OWOutputHigh;
   __delay_us(1);
}


/*******************************************************************************
 * FUNCTION:   DS1820_ReadByte
 * PURPOSE:    Reads a single byte from the DS1820 device.
 *
 * INPUT:      -
 * OUTPUT:     -
 * RETURN:     uint8          byte which has been read from the DS1820
 ******************************************************************************/
uint8_t OWReadByte(void)
{
   uint8_t i;
   uint8_t value = 0;

   for (i=0 ; i < 8; i++)
   {
      if ( OWReadBit() )
      {
         value |= (1 << i);
      }
   }
   return(value);
}


/*******************************************************************************
 * FUNCTION:   DS1820_WriteByte
 * PURPOSE:    Writes a single byte to the DS1820 device.
 *
 * INPUT:      val_u8         byte to be written
 * OUTPUT:     -
 * RETURN:     -
 ******************************************************************************/
void OWWriteByte(uint8_t val_u8)
{
   uint8_t i;
   uint8_t temp;

   for (i=0; i < 8; i++)      /* writes byte, one bit at a time */
   {
      temp = val_u8 >> i;     /* shifts val right 'i' spaces */
      temp &= 0x01;           /* copy that bit to temp */
      OWWriteBit(temp);  /* write bit in temp into */
   }
}

void docrc8(uint8_t Byte) {
    uint8_t n = 0; /* counter */
    uint8_t Bit; /* LSB */

    while (n < 8) /* 8 bits */ {
        /* XOR current LSB of input with CRC8's current X^8 */
        Bit = crc8 ^ Byte; /* XOR */
        Bit &= 0b00000001; /* filter LSB */

        /* shift CRC right */
        crc8 >>= 1; /* for next bit */

        if (Bit) /* XORed LSB is 1 */ {
            /*
             *  XOR CRC's X^5 and X^4 with 1
             *  - XOR with 0b00011000
             *  - since CRC is already shifted right: XOR with 0b00001100
             *  - since we have to feed the XORed LSB back into the CRC
             *    and the MSB is 0 after shifting: XOR with 0b10001100
             */

            crc8 ^= 0b10001100; /* XOR */
        }
        /*  when 0:
         *  - XOR would keep the original bits
         *  - MSB will be 0 after a right shift anyway
         */

        /* shift input right */
        Byte >>= 1; /* for next input bit */

        n++; /* next bit */
    }
}


bool OWSearch(void) {
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number;
    uint8_t id_bit, cmp_id_bit;
    unsigned char rom_byte_mask, search_direction;
    bool search_result;
    
    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = false;
    crc8 = 0;

    // if the last call was not the last one
    if (!LastDeviceFlag) {
        // 1-Wire reset
        if (!OWReset()) {
            // reset the search
            LastDiscrepancy = 0;
            LastDeviceFlag = false;
            LastFamilyDiscrepancy = 0;
            return false;
        }

        // issue the search command 
        OWWriteByte(0xF0);

        // loop to do the search
        do {
            // read a bit and its complement
            id_bit = OWReadBit();
            cmp_id_bit = OWReadBit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit; // bit write value for search
                else {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);
                    
                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0) {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                OWWriteBit(search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0) {
                    docrc8(ROM_NO[rom_byte_number]); // accumulate the CRC
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        } while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (!((id_bit_number < 65) || (crc8 != 0)))
            // if (!(id_bit_number < 65))
        {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)
                LastDeviceFlag = true;

            search_result = true;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0]) {
        LastDiscrepancy = 0;
        LastDeviceFlag = false;
        LastFamilyDiscrepancy = 0;
        search_result = false;
    }

    return search_result;
}


//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : no device present
//

bool OWFirst(void) {
    // reset the search state
    LastDiscrepancy = 0;
    LastDeviceFlag = false;
    LastFamilyDiscrepancy = 0;

    return OWSearch();
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return TRUE  : device found, ROM number in ROM_NO buffer
//        FALSE : device not found, end of search
//

bool OWNext(void) {
    // leave the search state alone
    return OWSearch();
}

void OWAddrDevice(uint8_t nAddrMethod)
{
   uint8_t i;
   
   if (nAddrMethod == OW_CMD_MATCHROM)
   {
      OWWriteByte(OW_CMD_MATCHROM);     /* address single devices on bus */
      for (i = 0; i < OW_ADDR_LEN; i ++)
      {
         OWWriteByte(ROM_NO[i]);
      }
   }
   else
   {
      OWWriteByte(OW_CMD_SKIPROM);     /* address all devices on bus */
   }
}



