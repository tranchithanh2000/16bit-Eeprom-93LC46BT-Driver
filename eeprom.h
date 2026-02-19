/* ============================================================================
 *  eeprom_93lc46bt.h
 * ============================================================================
 *  93LC46BT Microwire EEPROM Driver (bit-bang)
 *
 *  Device:
 *    - Organization: 64 words x 16 bits
 *    - Address: 6 bits (0..63)
 *    - Data: 16 bits
 *
 *  Protocol (Microwire 3-wire):
 *    - Signals: CS, SK, DI (MOSI), DO (MISO)
 *    - MSB first
 *
 *  Notes:
 *    - This driver does not depend on SPI peripheral (bit-bang GPIO).
 *    - Provide correct GPIO pin macros and delay to meet timing.
 *    - Ready/Busy polling: after WRITE, pulse SK with CS=HIGH, DO indicates ready.
 *
 *  Author: <Tran Chi Thanh>
 * ============================================================================
 */

#ifndef EEPROM_93LC46BT_H
#define EEPROM_93LC46BT_H

#include <stdint.h>
#include <stddef.h>

/* -------------------------------------------------------------------------- */
/*                                Configuration                               */
/* -------------------------------------------------------------------------- */

/* ------------------EEPROM command definitions -----------------------*/
#define     ERASE           0x0007  /* Erase                            */
#define     ERAL            0x0009  /* Erase All                        */
#define     EWDS            0x0001  /* Erase/Write Disable              */
#define     EWEN            0x0019  /* Erase/Write Enable               */
#define     READ            0x0003  /* Read                             */
#define     WRITE           0x0005  /* Write                            */
#define     WRAL            0x0011  /* Write All                        */

#define     NERASE          3       /* Erase                            */
#define     NERAL           9       /* Erase All                        */
#define     NEWDS           9       /* Erase/Write Disable              */
#define     NEWEN           9       /* Erase/Write Enable               */
#define     NREAD           3       /* Read                             */
#define     NWRITE          3       /* Write                            */
#define     NWRAL           9       /* Write All                        */

/*----------------------------------------------------------------------*/

#define     INITMARK0       0x5555  /* Initialization data 0            */
#define     INITMARK1       0xaaaa  /* Initialization data 1            */

/* ------------------EEPROM Functions Declaration -----------------------*/
void            EppInit(void);                                               /* Init EEPROM*/
void            EepSetCmd(unsigned int data, int cnt);                      /* Set EEPROM command */
void            EepSetAdd(unsigned int add);                                /* Set EEPROM address */
void            EepWrData(unsigned int data);                               /* Write EEPROM data */
unsigned int    EepRdData(void);                                            /* Read EEPROM data */
unsigned int    EepCheckRdy(void);                                          /* Check EEPROM ready */
unsigned int    EepWaitRdy(void);                                           /* Wait for EEPROM ready */
void            EepEnd(void);                                               /* End EEPROM access */

unsigned int    EepErase(unsigned int add);                                 /* Erase 1 word */
unsigned int    EepErAll(void);                                             /* Erase all EEPROM */
unsigned int    EepWrDis(void);                                             /* Disable write */
unsigned int    EepWrEnb(void);                                             /* Enable write */

unsigned int    EepRead(unsigned int add, unsigned int *buf);               /* Read 1 word */
unsigned int    EepMlRead(unsigned int add, unsigned int *buf, int cnt);    /* Multi-word read */
unsigned int    EepWrite(unsigned int add, unsigned int *buf);              /* Write 1 word */
unsigned int    EepWrNoWait(unsigned int add, unsigned int *buf);           /* Write 1 word (no wait) */
unsigned int    EepMlWrite(unsigned int add, unsigned int *buf, int cnt);   /* Multi-word write */
unsigned int    EepWriteAll(unsigned int *buf);                             /* Write all data */
unsigned int    EepTest(void);                                              /* EEPROM R/W check */

#endif