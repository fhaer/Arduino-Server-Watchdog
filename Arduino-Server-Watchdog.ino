#include <EEPROM.h>

int led = 13;
int relaisPin = 5;
unsigned long lastTime = 0;

void setup()
{
  pinMode(led, OUTPUT);  
  pinMode(relaisPin, OUTPUT);    
  Serial.begin(9600);
  Serial.setTimeout(500);
  Serial.println("Syntax: S<UnixTime> || P || D");
  mBlink(2);
}

void loop() 
{
   unsigned long currentTime = millis();
   if (currentTime - lastTime > 500) {
     Serial.println("READY_TO_RECEIVE");
     lastTime = millis();
   }
   if (Serial.available() > 0) {
    // get incoming byte
    int rd = Serial.read();
    if (rd == 'S') {
      // read unix time
      unsigned long utime = serialReadLong();
      Serial.println("SHUTDOWN_CONFIRMED");
      delay(30000);
      storeResetTime(utime);
      vdrReset();
    }
    if (rd == 'P') {
      printResetTimes();
    }
    if (rd == 'D') {
      deleteResetTimes();
    }
  }
}

void vdrReset() {
  Serial.println("RESET");
  digitalWrite(relaisPin, HIGH);   
  delay(100);               
  digitalWrite(relaisPin, LOW);      
}

void mBlink(int times) {
  for (int i=0; i<times; i++) {
    if (i != 0) {
      delay(100);       
    }
    digitalWrite(led, HIGH);   
    delay(100);               
    digitalWrite(led, LOW);          
  }
}

void deleteResetTimes() {
  Serial.println("Delete stored reset times");
  EEPROM.write(0, 0);
}

void printResetTimes() {
  Serial.print("Number of resets: ");
  unsigned int counter = EEPROM.read(0);
  Serial.println(counter);
  Serial.println("Reset times:");
  for (int i=0; i<counter; i++) {
    int address = 1 + i * 4;
    unsigned long utime = eepromReadLong(address);
    Serial.println(utime);
  }
}

void storeResetTime(unsigned long utime) {
  unsigned int counter = EEPROM.read(0);
  // reset if 255 unix times are stored (1 KB full)
  if (counter == 255) {
    counter = 0;
  }
  EEPROM.write(0, counter + 1);
  int address = 1 + counter * 4;
  eepromWriteLong(address, utime);
}

unsigned long serialReadLong() {
  unsigned long value = 0L;
  // a long value has up to 10 characters
  char buffer[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
  Serial.readBytes(buffer, 10);
  int exponent = 0;
  for (int i=9; i >= 0; i--) {
    // convert ascii to digit (ASCII 48 is a 0)
    int digit = buffer[i] - 48;
    if (digit >= 0 && digit <= 9) {
      value += digit * powInt(10, exponent);
      exponent++;
    }
  }
  return value;
}

unsigned long powInt(int base, int exponent) {
  unsigned long value = 1;
  for (int i=0; i<exponent; i++) {
    value *= base;
  }
  return value;
}

void eepromWriteLong(int address, unsigned long value) {
  EEPROM.write(address+3,lowByte(value));
  EEPROM.write(address+2,highByte(value));
  value = value >> 16;
  EEPROM.write(address+1,lowByte(value));
  EEPROM.write(address,highByte(value));
}

unsigned long eepromReadLong(int address) {
  unsigned long l = 0L;
  byte b = B0;
  b = EEPROM.read(address);
  l = b << 8;
  b = EEPROM.read(address+1);
  l = l | b;
  l = l << 8;
  b = EEPROM.read(address+2);
  l = l | b;
  l = l << 8;
  b = EEPROM.read(address+3);
  l = l | b;
  return l;
}

