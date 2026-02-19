#include "eeprom.h"


typedef unsigned int    uint;       

/* -------------------------------------------------------------------------- */
/*                          GPIO Low-level Control                            */
/* -------------------------------------------------------------------------- */
/* Serial Clock */
static inline void EE_SK_LOW(void)  { DL_GPIO_clearPins(GPIO_PORTA_PORT, IO_SCK); }
static inline void EE_SK_HIGH(void) { DL_GPIO_setPins  (GPIO_PORTA_PORT, IO_SCK); }

/* Serial Data Input (MCU -> EEPROM) */
static inline void EE_DI_LOW(void)  { DL_GPIO_clearPins(GPIO_PORTB_PORT, IO_SDI); }
static inline void EE_DI_HIGH(void) { DL_GPIO_setPins  (GPIO_PORTB_PORT, IO_SDI); }

/* Chip Select */
static inline void EE_CS_LOW(void)  { DL_GPIO_clearPins(GPIO_CS_PORT, EEPROM_CS); }
static inline void EE_CS_HIGH(void) { DL_GPIO_setPins  (GPIO_CS_PORT, EEPROM_CS); }

/* Serial Data Output (EEPROM -> MCU) */
static inline uint EE_DO_READ(void)	{return (DL_GPIO_readPins(GPIO_PORTA_PORT, IO_SDO) ? 1u : 0u);}

/* Please prefer using your delay method such as (Timer, System Tick) 
   The ee_delay function below is using for test purpose
*/
static inline void ee_delay(void)
{
    volatile int i;
    for(i = 0; i < 500; i++);
}


/* -------------------------------------------------------------------------- */
/*                          Microwire Bit-Level Routines                      */
/* -------------------------------------------------------------------------- */
/**
 * @brief Initialize GPIO to safe idle state (CS low, SK low, DI low).
 */
void EppInit(void){
	EE_CS_LOW();
	EE_SK_LOW();
	EE_DI_LOW();
}

/**
 * @brief Shift out command bits (MSB-first)
 * 
 * @param data Command data
 * @param cnt  Number of bits to transmit
 */
void EepSetCmd(unsigned data, int cnt)
{
    for(int loop = cnt - 1; loop >= 0; loop--)
    {
        EE_SK_LOW();  ee_delay();

        if(data & (1u << loop)) EE_DI_HIGH();
        else                    EE_DI_LOW();

        ee_delay();
        EE_SK_HIGH(); ee_delay();
    }

    EE_SK_LOW(); ee_delay();
}

/**
 * @brief Shift out 6-bit address (MSB-first)
 */
void EepSetAdd(uint add)
{
    for(int loop = 5; loop >= 0; loop--)
    {
        EE_SK_LOW(); ee_delay();

        if(add & (1u << loop)) EE_DI_HIGH();
        else                   EE_DI_LOW();

        ee_delay();
        EE_SK_HIGH(); ee_delay();
    }

    EE_SK_LOW(); ee_delay();
}

/**
 * @brief Transmit 16-bit data word (MSB-first)
 */
void EepWrData(uint data)
{
    for(int loop = 15; loop >= 0; loop--)
    {
        EE_SK_LOW(); ee_delay();

        if(data & (1u << loop)) EE_DI_HIGH();
        else                    EE_DI_LOW();

        ee_delay();
        EE_SK_HIGH(); ee_delay();
    }

    EE_SK_LOW(); ee_delay();
}

/**
 * @brief Read 16-bit data word from EEPROM
 * 
 * Sampling strategy:
 *   - SK rising edge triggers EEPROM shift
 *   - Data sampled after clock transition
 */
uint EepRdData(void)
{
    uint tmp = 0;

    EE_DI_LOW();

    for(int loop = 0; loop < 16; loop++)
    {
        EE_SK_HIGH(); ee_delay();

        EE_SK_LOW(); ee_delay();

        tmp = (tmp << 1) | EE_DO_READ();
    }

    return tmp;
}


/* -------------------------------------------------------------------------- */
/*                          EEPROM Control Operations                         */
/* -------------------------------------------------------------------------- */
/**
 * @brief Wait until internal write cycle completes
 *
 * Microwire Ready/Busy polling:
 *   - After WRITE, CS high
 *   - Pulse SK
 *   - DO = 0 → Busy
 *   - DO = 1 → Ready
 *
 * @return 1 if ready, 0 if timeout
 */
uint EepWaitRdy(void)
{
    EE_DI_LOW();
    EE_SK_LOW(); ee_delay();

    EE_CS_HIGH(); ee_delay();

    for(int loop = 0; loop < 40000; loop++)
    {
        EE_SK_HIGH(); ee_delay();
        if(EE_DO_READ())
        {
            EE_SK_LOW();  ee_delay();
            EE_CS_LOW();  ee_delay();
            return 1;
        }
        EE_SK_LOW(); ee_delay();
    }

    EE_CS_LOW(); ee_delay();
    return 0;
}

/**
 * @brief Enable write operations (EWEN command)
 *
 * Command format:
 *   1 + 00 + 11 + xxxx
 */
uint EepWrEnb(void)
{
    EE_SK_LOW(); ee_delay();
    EE_CS_HIGH(); ee_delay();

    EepSetCmd(0b1, 1);       // start
    EepSetCmd(0b00, 2);      // opcode
    EepSetCmd(0b11, 2);      // EWEN
    EepSetCmd(0b0000, 4);    // don't care

    EE_CS_LOW(); ee_delay();
    EE_DI_LOW(); ee_delay();
    return 1;
}

/**
 * @brief Disable write operations (EWDS command)
 *
 * Command format:
 *   1 + 00 + 00 + xxxx
 */
uint EepWrDis(void)
{
    EE_SK_LOW(); ee_delay();
    EE_CS_HIGH(); ee_delay();

    EepSetCmd(0b1, 1);       // start
    EepSetCmd(0b00, 2);      // opcode
    EepSetCmd(0b00, 2);      // EWDS
    EepSetCmd(0b0000, 4);    // don't care

    EE_CS_LOW(); ee_delay();
    EE_DI_LOW(); ee_delay();
    return 1;
}

/**
 * @brief Read one 16-bit word from EEPROM
 *
 * Command format:
 *   1 + 10 + A5..A0 + DATA(16)
 *
 * @param add Address (0–63)
 * @param buf Output buffer
 */
uint EepRead(uint add, uint *buf)
{
    EE_SK_LOW(); ee_delay();
    EE_CS_HIGH(); ee_delay();

    EepSetCmd(0b1, 1);       // start
    EepSetCmd(0b10, 2);      // READ
    EepSetAdd(add & 0x3F);   // 6-bit address

    *buf = EepRdData();

    EE_CS_LOW(); ee_delay();
    EE_DI_LOW(); ee_delay();
    return 1;
}

/**
 * @brief Write one 16-bit word to EEPROM
 *
 * Command format:
 *   1 + 01 + A5..A0 + DATA(16)
 *
 * Flow:
 *   1. Enable write
 *   2. Issue WRITE command
 *   3. Wait ready
 *   4. Disable write
 *
 * @param add Address (0–63)
 * @param buf Input data
 */
uint EepWrite(uint add, uint *buf)
{
    uint result;

	EepWrEnb();

    EE_SK_LOW(); ee_delay();
    EE_CS_HIGH(); ee_delay();

    EepSetCmd(0b1, 1);       // start
    EepSetCmd(0b01, 2);      // WRITE
    EepSetAdd(add & 0x3F);
    EepWrData(*buf);

    EE_CS_LOW(); ee_delay();
    EE_DI_LOW(); ee_delay();

    result = EepWaitRdy();

	EepWrDis();
    return result;
}


/* -------------------------------------------------------------------------- */
/*                               Debug Utility                                */
/* -------------------------------------------------------------------------- */

/**
 * @brief EEPROM self-test
 *
 * - Write sequential test pattern
 * - Read back
 * - Compare result
 * - Print mismatch
 */
int val = 0;
void EEP_WriteReadTest()
{
    uint	wr_data[64] = {0};
	uint	re_data[64] = {0};
    uint    ii;

	EppInit();			//Init Eeprom
	
	while(1){
		// Reset Data
		for(ii=0; ii<64; ii++)      wr_data[ii] = 0x0000;

		// Assign Data  
		for(int i = 0; i < 64; i++) {wr_data[i] = val + i; val++;}

		// Eeprom Write
		for(ii=0; ii<64; ii++) EepWrite(ii, &wr_data[ii]);

		// Eeprom Read
		for(ii=0; ii<64; ii++) EepRead(ii, &re_data[ii]);
		
		// Verify Data
		int match = 1;    // Error flag
		for (int i = 0; i < 64; i++){
			if (re_data[i] != wr_data[i]){ match = 0; break; }
		}

		for (int i = 0; i < 64; i++){
			if (wr_data[i] != 0 || re_data[i] != 0){
				printf("i=%02d  W=%04X  R=%04X\r\n", i, wr_data[i], re_data[i]);
			}
		}
	}
}

