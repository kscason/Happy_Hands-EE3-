#include<Wire.h>
#define bluetooth Serial1

#define Upright_X 50
#define Upright_Y -5
#define Upright_Z 0
#define Sideways_X 14
#define Sideways_Y -39
#define Sideways_Z 30
#define Downwards_X 10
#define Downwards_Y -37
#define Downwards_Z -37
#define OFFSET 20

unsigned contactPin[] = {23, 22, 21, 12, 11, 10, 9, 8, 7, 6}; // Pick digital pins for contacts
unsigned flexSensorPin[] = {A6, A3, A2, A1, A0}; // Flex sensor analog pins
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;  // Store the MPU data
int16_t AcX_SCALED, AcY_SCALED, AcZ_SCALED;
int flexValue[5];
unsigned i, j;  // Used for iterating

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // Set all contact pins to INPUT
  for (i = 0; i < (sizeof(contactPin)/sizeof(int)); i++) {
    pinMode(contactPin[i], INPUT_PULLUP);
  }

  // Set all flex sensor pins to INPUT
  for (i = 0; i < (sizeof(flexSensorPin)/sizeof(int)); i++) {
    pinMode(flexSensorPin[i], INPUT_PULLUP);
  }
  
  Serial.begin(115200);  // Begin the serial monitor at 115200bps
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
}

void loop(){
  // Request MPU motion data
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);  // request a total of 14 registers

  // Read accelerometer/gyroscope data
  AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  AcX_SCALED = map(AcX, -32768, 32767, -100, 100);
  AcY_SCALED = map(AcY, -32768, 32767, -100, 100);
  AcZ_SCALED = map(AcZ, -32768, 32767, -100, 100);
  
  Serial.print("AcX = "); Serial.print(AcX_SCALED);
  Serial.print(" | AcY = "); Serial.print(AcY_SCALED);
  Serial.print(" | AcZ = "); Serial.print(AcZ_SCALED);
  Serial.print(" | Tmp = "); Serial.println(Tmp / 340.00 + 36.53);  //equation for temperature in degrees C from datasheet
  Serial.print("GyX = "); Serial.print(map(GyX, -32768, 32767, -100, 100));
  Serial.print(" | GyY = "); Serial.print(map(GyY, -32768, 32767, -100, 100));
  Serial.print(" | GyZ = "); Serial.println(map(GyZ, -32768, 32767, -100, 100));

  if (AcX_SCALED <= Upright_X + OFFSET && AcX_SCALED >= Upright_X - OFFSET
    && AcY_SCALED <= Upright_Y + OFFSET && AcY_SCALED >= Upright_Y - OFFSET
    && AcZ_SCALED <= Upright_Z + OFFSET && AcZ_SCALED >= Upright_Z - OFFSET)
    Serial.println("Upright");
  else if (AcX_SCALED <= Sideways_X + OFFSET && AcX_SCALED >= Sideways_X - OFFSET
    && AcY_SCALED <= Sideways_Y + OFFSET && AcY_SCALED >= Sideways_Y - OFFSET
    && AcZ_SCALED <= Sideways_Z + OFFSET && AcZ_SCALED >= Sideways_Z - OFFSET)
    Serial.println("Sideways");
  else if (AcX_SCALED <= Downwards_X + OFFSET && AcX_SCALED >= Downwards_X - OFFSET
    && AcY_SCALED <= Downwards_Y + OFFSET && AcY_SCALED >= Downwards_Y - OFFSET
    && AcZ_SCALED <= Downwards_Z + OFFSET && AcZ_SCALED >= Downwards_Z - OFFSET)
    Serial.println("Downwards");
  else
    Serial.println("Neutral");
  
/*

  Serial.println("Contact data:");

  // Write one pin as high and read from the others
  for (i = 0; i < (sizeof(contactPin)/sizeof(int)); i++) { // sizeOf returns number of bytes
    pinMode(contactPin[i], OUTPUT);
    digitalWrite(contactPin[i], LOW);
    
    // Used to allow pullup resistors to settle
    delayMicroseconds(50);
    
    for (j = 0; j < (sizeof(contactPin)/sizeof(int)); j++) { 
      if ( i != j )
        Serial.print(digitalRead(contactPin[j]));
      else
        Serial.print("1");
      Serial.print("  ");
    }
    pinMode(contactPin[i], INPUT_PULLUP);
    Serial.println();
  }
  
  Serial.println("Flex data:");
  Serial.print("Thumb:");
  Serial.print("\t");
  Serial.print("Index:");
  Serial.print("\t");
  Serial.print("Middle:");
  Serial.print("\t");
  Serial.print("Ring:");
  Serial.print("\t");
  Serial.print("Pinky:");
  Serial.println("\t");
  
  for (i = 0; i < (sizeof(flexSensorPin)/sizeof(int)); i++) {
    flexValue[i] = analogRead(flexSensorPin[i]);
    Serial.print(flexValue[i]);
    Serial.print("\t");
  }

  Serial.println();

  if (flexValue[0] <= 460)
    Serial.print("Straight");
  else
    Serial.print("Bent");
  Serial.print("\t");

  if (flexValue[1] <= 460)
    Serial.print("Straight");
  else if (flexValue[1] <= 540)
    Serial.print("Curled");
  else
    Serial.print("Bent");
  Serial.print("\t");

  if (flexValue[2] <= 570)
    Serial.print("Straight");
  else
    Serial.print("Bent");

  Serial.print("\t");

  if (flexValue[3] <= 480)
    Serial.print("Straight");
  else
    Serial.print("Bent");

  if (flexValue[4] <= 500)
    Serial.print("Straight");
  else
    Serial.print("Bent");
      */

//  Serial.println();
  Serial.println();

  delay(10);
}
