#ifndef _RFM01_H_
#define _RFM01_H_

#include <stdint.h>

void rfm01_setup(void);
void rfm01_reset(void);
void rfm01_ctrl_write(uint16_t cmd);
uint8_t rfm01_status_read(void);
uint8_t rfm01_fifo_read(void);


#define NRES_PIN        8/* RESET pin(active low) */
#define NFFS_PIN        9   /* CMD/FIFO select(high/low) */
#define NSET_PIN        10  /* Chip select */

#define FFIT_PIN        12  /* FIFO OUT DATA(MISO) */

#define SDI_PIN         11  /* CMD IN (MOSI) */
#define SCK_PIN         13  /* CMD/FIFO clock */

#define RFM01_STATUS_FIFO_IRQ 0x80


#endif

