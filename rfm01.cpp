#include <arduino.h>
#include <SPI.h>
#include "rfm01.h"



//MSB bit first
//Rising edge read/write
//Read fifo status: 4 clk, SDI=low: FFIT high=> new data (max rate:1ms)
//Read fifi: NFFS low then NSET low, read 8bits from FFIT   (50% clk)
//Write 16bit config: NFFS high then NSET low, write 16bit to SDI
//Incomming data:16byte => 8byte (Manchester).



void rfm01_setup(void){
  digitalWrite(NSET_PIN, HIGH);
  digitalWrite(NFFS_PIN, HIGH);
  digitalWrite(NRES_PIN, HIGH);
  
  pinMode(NSET_PIN, OUTPUT);
  pinMode(NFFS_PIN, OUTPUT);
  pinMode(NRES_PIN, OUTPUT);

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV64); //250khz
  SPI.setDataMode(SPI_MODE0);
}

void rfm01_reset(void){
    digitalWrite(NRES_PIN, HIGH);
    delay(5);

    /* Pulse reset */
    digitalWrite(NRES_PIN, LOW);
    delay(50);
    digitalWrite(NRES_PIN, HIGH);
}

void rfm01_ctrl_write(uint16_t cmd){
  digitalWrite(NFFS_PIN, HIGH); //command
  
  digitalWrite(NSET_PIN, LOW);
  SPI.transfer16(cmd);
  digitalWrite(NSET_PIN, HIGH);

}

uint8_t rfm01_status_read(void){
  uint8_t status;
  
  digitalWrite(NFFS_PIN, HIGH); //Command
  digitalWrite(NSET_PIN, LOW);
  status = SPI.transfer(0x00);
  digitalWrite(NSET_PIN, HIGH);
  
  return status;
}

uint8_t rfm01_fifo_read(void){
  uint8_t status;
  
  digitalWrite(NFFS_PIN, LOW); //FIFO
  digitalWrite(NSET_PIN, LOW);
  status = SPI.transfer(0x00);
  digitalWrite(NSET_PIN, HIGH);
  
  return status;
}


