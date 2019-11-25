#include <Wire.h>

#define WRITE 2
#define READY 3
#define BURN 4
#define WRITE_ENABLE 5
#define ADDRESS 0x50

class I2C_EEPROM
{
private:
  int address;

  void send_address(uint16_t eeprom_address)
  {
    Wire.beginTransmission(address & 0xfe); 
    // mask with binary [1111 1110]
    // this sets the last bit of the I2C address to [1]
    // which is the I2C write mode.

    
    Wire.write(eeprom_address >> 8);
    // write the top address byte first
    // [XXXX XXXX XXXX XXXX]
    //  ^^^^ ^^^^

    Wire.write(eeprom_address & 0xff);
    
  }
  
public:
  I2C_EEPROM(byte _address)
  {
    address = _address;
  }

  void burn(uint16_t eeprom_address,byte data)
  {
    send_address(eeprom_address);
    Wire.write(data); // write data
    Wire.endTransmission();
    //Serial.println(Wire.endTransmission()); // end data transmission

    delay(5); // EEPROM write take around 5ms
  }

  byte read(uint16_t eeprom_address)
  {
    byte data = 0xFF;
    
    send_address(eeprom_address);
    Wire.endTransmission();
    //Serial.println(Wire.endTransmission());
    
    Wire.requestFrom(address & 0xfe, 1);
    if (Wire.available()) {
      //Serial.println("reading data..");
      data = Wire.read();
    }

    return data;
  }

  void burn_block(uint16_t eeprom_address, byte* data, uint16_t count)
  {
    for(uint16_t i = 0; i < count; i ++) 
      burn(eeprom_address + i, data[i]);
  }
};

I2C_EEPROM eeprom(ADDRESS);

void setup() {
  digitalWrite(WRITE_ENABLE, HIGH);
  
  pinMode(WRITE, OUTPUT);
  pinMode(READY, OUTPUT); 
  pinMode(BURN, INPUT_PULLUP);
  pinMode(WRITE_ENABLE, OUTPUT);
  digitalWrite(WRITE_ENABLE, LOW);
  
  Wire.begin();
  Serial.begin(115200);

  digitalWrite(READY, HIGH);
}

void burn(byte* data, uint16_t count)
{
  while(digitalRead(BURN) != LOW);
  Serial.println("Burning data...");
  digitalWrite(READY, LOW);
  digitalWrite(WRITE, HIGH);
  delay(10);
  eeprom.burn_block(0x0000, data, count);
  delay(10);
  
  for(int i = 0; i < 32; i++)
  {
    Serial.print(i * 0x10, HEX);
    Serial.print(": ");
    for(int j = 0; j < 16; j++)
    {
      Serial.print(eeprom.read(i * 0x10 + j), HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  
  digitalWrite(READY, HIGH);
  digitalWrite(WRITE, LOW);
}

uint16_t count = 64;
byte data[64] = {
  0x56, 0x45, 0x86, 0x98, 0xb6, 0x40, 0xa9, 0x37, 0x29, 0x8c, 0x9c, 0x6c, 0x24, 0x91, 0xb8, 0x04,
  0xc5, 0x7d, 0x84, 0x58, 0xea, 0x51, 0x5c, 0x0c, 0xc4, 0x3a, 0xa7, 0x49, 0x1b, 0x8e, 0xfd, 0x7b,
  0xa4, 0x25, 0x4c, 0x40, 0xd1, 0xeb, 0x14, 0xa4, 0x29, 0x20, 0x4f, 0x5e, 0x94, 0x39, 0xf9, 0x74,
  0x8e, 0x03, 0xa6, 0x29, 0x81, 0x68, 0xdd, 0x2e, 0xb8, 0x57, 0x59, 0x6a, 0xca, 0x49, 0x64, 0x5b 
};

void loop() {
  for(int i = 0; i < 32; i++)
  {
    Serial.print(i * 0x10, HEX);
    Serial.print(": ");
    for(int j = 0; j < 16; j++)
    {
      Serial.print(eeprom.read(i * 0x10 + j), HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  while(1);
  burn(data, count);
}
