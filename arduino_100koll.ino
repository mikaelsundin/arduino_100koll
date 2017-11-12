#include <SPI.h>
#include <stdint.h>

#include "rfm01.h"

/* Default settings loaded by LCD */
void rfm01_default(void){
  rfm01_ctrl_write(0x892D);
  rfm01_ctrl_write(0xE196);
  rfm01_ctrl_write(0xCC0E);
  rfm01_ctrl_write(0xC69F);
  rfm01_ctrl_write(0xC46A);
  rfm01_ctrl_write(0xC88A);
  rfm01_ctrl_write(0xC080);
  rfm01_ctrl_write(0xCE88);
  rfm01_ctrl_write(0xCD8B);
  rfm01_ctrl_write(0xC081);
  rfm01_ctrl_write(0xC200);
  rfm01_ctrl_write(0xA618);
  rfm01_ctrl_write(0xCE89);
  rfm01_ctrl_write(0xCE8B);
}

void setup(void){
  Serial.begin(115200);
  
  rfm01_setup();
  rfm01_reset();

  rfm01_default();

}

/* Decode manchester to 4bits */
uint8_t decode_nibble(uint8_t code){
  uint8_t value;
  switch(code){
  case 0b01010101:
    value=0u;
    break;
  case 0b01010110:
    value=1u;
    break;
  case 0b01011001:
    value=2u;
    break;
  case 0b01011010:
    value=3u;
    break;
  case 0b01100101:
    value=4u;
    break;
  case 0b01100110:
    value=5u;
    break;
  case 0b01101001:
    value=6u;
    break;
  case 0b01101010:
    value=7u;
    break;
    
 case 0b10010101:
    value=8u;
    break;
  case 0b10010110:
    value=9u;
    break;
  case 0b10011001:
    value=10u;
    break;
  case 0b10011010:
    value=11u;
    break;
  case 0b10100101:
    value=12u;
    break;
  case 0b10100110:
    value=13u;
    break;
  case 0b10101001:
    value=14u;
    break;
  case 0b10101010:
    value=15u;
    break;
  default:
    value = 0xff;
    break;
  }
  return value;
}

/* Parse payload */
void parse(uint8_t data[8]){
  uint8_t pairing               = (data[0] & 0x80) ? 0x01 : 0x00;
  uint8_t data_sensor_indicator = (data[0] & 0x40) ? 0x01 : 0x00;
  uint16_t address = (data[0] & 0x0F)<< 8 | data[1];
  uint8_t i;
  
  
  if(data_sensor_indicator == 0x00){
      /* Electricity data */
      uint8_t sensor_valid[3];
      uint8_t sensor_value[3];
  
      /* sensor 1 */
      sensor_valid[0] = data[2] & 0x80;                   //sensor valid
      sensor_value[0] =(data[2]<<8 | data[3]) & 0x7FFFu;  //15 bit value

      /* sensor 2 */
      sensor_valid[1] = (data[4] & 0x80);                 //sensor valid
      sensor_value[1] = (data[4]<<8 | data[5]) & 0x7FFFu; //15 bit value

      /* sensor 3 */
      sensor_valid[2] = (data[6] & 0x80);                 //sensor valid
      sensor_value[2] = (data[6]<<8 | data[7]) & 0x7FFFu; //15 bit value

      Serial.print("{");
      Serial.print("\"timestamp\":");
      Serial.print(millis());
      Serial.print(",\"sensor\":");
      Serial.print(address);
      Serial.print("");
      /* print out valid sensors */
      for(i=0;i<3;i++){
        if(sensor_valid[i] != 0u){
          Serial.print(",\"power");
          Serial.print(i);
          Serial.print("\":");
          Serial.print(sensor_value[i]);
        }
      }
      Serial.println("}");
  }else{
    uint8_t sensor_type = data[3];
    uint32_t counter = 0u;

    for(i=0;i<4;i++){
      counter <<= 8;
      counter |= data[4+i];
    }

    /* Sensor types: 2:Electric, 3:Gas, 4:water */
    if(sensor_type == 2 || sensor_type == 3 || sensor_type == 4){
      Serial.print("{");
      Serial.print("\"timestamp\":");
      Serial.print(millis());
      Serial.print(",\"sensor\":");
      Serial.print(address);
      Serial.print("");

      if(sensor_type == 2){
        Serial.print(",\"electric\":");
      }else if(sensor_type == 3){
        Serial.print(",\"gas\":");
      }else if(sensor_type == 3){
        Serial.print(",\"water\":");
      }

      Serial.print(counter);
      Serial.println("}");

    }
      
  }
}

/* Decode incomming manchester packet */
void decode(uint8_t packet[16]){
  uint8_t data[8];
  
   /* Decode manchester */
  for(uint8_t i=0;i<8;i++){
     uint8_t upper = decode_nibble(packet[i*2u + 0u]);
     uint8_t lower = decode_nibble(packet[i*2u + 1u]);

     /* decode manchester and check if correct */
     data[i] = (upper<<4) | lower;
     if( (upper >= 0x10) || (lower >= 0x10)){
       return;
     }
     
     
  }

  /* parse data */
  parse(data);
}

void loop(void){
  static uint8_t index=0u;
  static uint8_t packet[16];
  static uint16_t timeout=0u;
  
  /* check if some exist on fifo */
  if(rfm01_status_read() & RFM01_STATUS_FIFO_IRQ){
    packet[index++] = rfm01_fifo_read();
    
    if(index >= 16){
      timeout = 0u;
      index=0u;
      decode(packet);
      rfm01_default();
    }
  }

  /* reset radio if no packets for ~15sec */
  if(timeout++ >= 15000u){
    timeout = 0u;
    index   = 0u;
    rfm01_default();
  }

  /* Rate limit */
  delayMicroseconds(1000);
  
}

