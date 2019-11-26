#include <Wire.h>
#define WRITE 4
#define READY 3
#define BURN 2
#define WRITE_ENABLE 5
#define ADDRESS 0x50

// class for an I2C EEPROM chip
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
    // [XXXX XXXX XXXX XXXX]
    //            ^^^^ ^^^^
  }
public:
  I2C_EEPROM(byte _address)
  {
    address = _address;
  }
  
  void burn(uint16_t eeprom_address,byte data)
  {
    send_address(eeprom_address); // send the eeprom addres to eeprom
    Wire.write(data); // write data

    int ret = Wire.endTransmission(); // end data transmission
    
    if(ret) // error handling
    {
      Serial.print("burn error: ");
      Serial.println(ret);
    }
    
    delay(5); // EEPROM write take around 5ms
  }
  
  byte read(uint16_t eeprom_address)
  {
    byte data = 0xFF; // default value
    send_address(eeprom_address); // send address
    int ret = Wire.endTransmission(); // end data transmission
    
    if(ret) // error handling
    {
      Serial.print("read error: ");
      Serial.println(ret);
    }
    
    Wire.requestFrom(address & 0xfe, 1);
    if (Wire.available()) {
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

// actuall start of program
I2C_EEPROM eeprom(ADDRESS);

void setup() {
  digitalWrite(WRITE_ENABLE, HIGH); // set up the write-enable pin (active low) to high
  pinMode(WRITE, OUTPUT); // led for WRITE
  pinMode(READY, OUTPUT); // led for READY
  pinMode(BURN, INPUT_PULLUP); // button input for WRITE
  pinMode(WRITE_ENABLE, OUTPUT); // set WP to low

  Wire.begin(); // begin I2C
  Serial.begin(115200); // init UART

  digitalWrite(READY, HIGH); // it's READY!
}

void print_hex(byte b)
{
  char buf[5];
  sprintf(buf, "0x%02X ", b);
  Serial.print(buf);
}

void dump(uint16_t count)
{
  int top = ceil(count / 16);
  for(int i = 0; i < top; i++)
  {
    char buf[10];
    sprintf(buf, "0x%04X: ", i * 0x10);
    Serial.print(buf);
    
    for(int j = 0; j < 16; j++)
      print_hex(eeprom.read(i * 0x10 + j));
    Serial.println();
  }
}

void burn(byte* data, uint16_t count)
{
  while(digitalRead(BURN) != LOW);
  digitalWrite(WRITE_ENABLE, LOW);
  Serial.println("Burning data...");
  digitalWrite(READY, LOW);
  digitalWrite(WRITE, HIGH);
  delay(10);
  eeprom.burn_block(0x0000, data, count);
  delay(10);

  dump(count);

  digitalWrite(READY, HIGH);
  digitalWrite(WRITE, LOW);
}

uint16_t count = 64;
byte data[64] = {
  0xaa, 0x45, 0x86, 0x98, 0xb6, 0x40, 0xa9, 0x37, 0x29, 0x8c, 0x9c, 0x6c, 0x24, 0x91, 0xb8, 0x04,
  0xc5, 0x7d, 0x84, 0x58, 0xea, 0x51, 0x5c, 0x0c, 0xc4, 0x3a, 0xa7, 0x49, 0x1b, 0x8e, 0xfd, 0x7b,
  0xa4, 0x25, 0x4c, 0x40, 0xd1, 0xeb, 0x14, 0xa4, 0x29, 0x20, 0x4f, 0x5e, 0x94, 0x39, 0xf9, 0x74,
  0x8e, 0x03, 0xa6, 0x29, 0x81, 0x68, 0xdd, 0x2e, 0xb8, 0x57, 0x59, 0x6a, 0xca, 0x49, 0x64, 0x5b 
};
void loop() {
  burn(data, count);
}
