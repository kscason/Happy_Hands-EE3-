#include<Wire.h>
#define bluetooth Serial1

#define Gy_THRESHOLD 25000
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

/*
 * Program sends data vector over Bluetooth to computer for processing into dictionary and also pretty prints to serial
 * NOTE: ASL sign vector terminated with newline '\n'
 */

unsigned contactPin[] = {23, 22, 21, 12, 11, 10, 9, 8, 7, 2}; // Pick digital pins for contacts
unsigned flexSensorPin[] = {A6, A3, A2, A1, A0}; // Flex sensor analog pins
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int contactVal;
int16_t AcX_SCALED, AcY_SCALED, AcZ_SCALED;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;  // Store the MPU data
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

//  Serial.begin(115200);  // Begin the serial monitor at 115200bps
  Serial.begin(115200);  // The Bluetooth Mate defaults to 115200bps
}

void loop(){
//  Serial.write(27); 
//  Serial.print("[2J"); // clear screen 
//  Serial.write(27); // ESC 
//  Serial.print("[H"); // cursor to home

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

  // Send movement and orientation data
  AcX_SCALED = map(AcX, -32768, 32767, -100, 100);
  AcY_SCALED = map(AcY, -32768, 32767, -100, 100);
  AcZ_SCALED = map(AcZ, -32768, 32767, -100, 100);
  
//  Serial.print("AcX = "); Serial.print(AcX_SCALED);
//  Serial.print(" | AcY = "); Serial.print(AcY_SCALED);
//  Serial.print(" | AcZ = "); Serial.print(AcZ_SCALED);
//  Serial.print(" | Tmp = "); Serial.print(Tmp / 340.00 + 36.53);  //equation for temperature in degrees C from datasheet
//  Serial.print("GyX = "); Serial.print(map(GyX, -32768, 32767, -100, 100));
//  Serial.print(" | GyY = "); Serial.print(map(GyY, -32768, 32767, -100, 100));
//  Serial.print(" | GyZ = "); Serial.println(map(GyZ, -32768, 32767, -100, 100));

  // Check for motion
  if (abs(GyX) >= Gy_THRESHOLD || abs(GyY) >= Gy_THRESHOLD || abs(GyZ) >= Gy_THRESHOLD) {
//    Serial.print("Movement: Moving\t");
    Serial.print("M");
  }
  else {
//    Serial.print("Movement: Stactic\t");
    Serial.print("S");
  }

  if (AcX_SCALED >= Upright_X - OFFSET
    && AcY_SCALED <= Upright_Y + OFFSET && AcY_SCALED >= Upright_Y - OFFSET
    && AcZ_SCALED <= Upright_Z + OFFSET && AcZ_SCALED >= Upright_Z - OFFSET) {
//    Serial.println("Orientation: Upright");
    Serial.print("U");
  }
  else if (AcX_SCALED <= Sideways_X + OFFSET && AcX_SCALED >= Sideways_X - OFFSET
    && AcY_SCALED <= Sideways_Y + OFFSET && AcY_SCALED >= Sideways_Y - OFFSET
    && AcZ_SCALED <= Sideways_Z + OFFSET && AcZ_SCALED >= Sideways_Z - OFFSET) {
//    Serial.println("Orientation: Sideways");
    Serial.print("S");
  }
  else if (AcX_SCALED <= Downwards_X + OFFSET && AcX_SCALED >= Downwards_X - OFFSET
    && AcY_SCALED <= Downwards_Y + OFFSET && AcY_SCALED >= Downwards_Y - OFFSET
    && AcZ_SCALED <= Downwards_Z + OFFSET && AcZ_SCALED >= Downwards_Z - OFFSET) {
//    Serial.println("Orientation: Downwards");
    Serial.print("D");
  }
  else {
//    Serial.println("Orientation: Neutral");
    Serial.print("N");
  }
  
  // Set flex sensor data
  for (i = 0; i < (sizeof(flexSensorPin)/sizeof(int)); i++) {
    flexValue[i] = analogRead(flexSensorPin[i]);
  }

  // Thumb (flexValue[0])
//  Serial.print("Thumb: ");
  if (flexValue[0] <= 460) {
//    Serial.print("Straight\t\t");
    Serial.print("S");
  }
  else {
//    Serial.print("Not straight\t");
    Serial.print("N");
  }
//  Serial.print(flexValue[0]);
//  Serial.println();

  // Index (flexValue[1])
//  Serial.print("Index: ");
  if (flexValue[1] <= 460) {
//    Serial.print("Straight\t\t");
    Serial.print("S");
  }
  else if (flexValue[1] <= 540) {
//    Serial.print("Curled\t\t");
    Serial.print("C");
  }
  else {
//    Serial.print("Bent\t\t");
    Serial.print("B");
  }
//  Serial.print(flexValue[1]);
//  Serial.println();

  // Middle (flexValue[2])
//  Serial.print("Middle: ");
  if (flexValue[2] <= 570) {
//    Serial.print("Straight\t");
    Serial.print("S");
  }
  else {
//    Serial.print("Not straight\t");
    Serial.print("N");
  }
//  Serial.print(flexValue[2]);
//  Serial.println();

  // Ring (flexValue[3])
//  Serial.print("Ring: ");
  if (flexValue[3] <= 480) {
//    Serial.print("Straight\t\t");
    Serial.print("S");
  }
  else {
//    Serial.print("Not Straight\t");
    Serial.print("N");
  }
//  Serial.print(flexValue[3]);
//  Serial.println();

  // Pinkie (flexValue[4])
//  Serial.print("Pinky: ");
  if (flexValue[4] <= 500) {
//    Serial.print("Straight\t\t");
    Serial.print("S");
  }
  else {
//    Serial.print("Not straight\t");
    Serial.print("N");
  }
//  Serial.print(flexValue[4]);
//  Serial.println();

  // For display purposes
//  Serial.print("\t");
  for (i = 0; i < (sizeof(contactPin)/sizeof(int)); i++) { // sizeOf returns number of bytes
//    Serial.print(i + 1);
//    Serial.print("  ");
  }
//  Serial.println();

  // Write one pin as high and read from the others
  for (i = 0; i < (sizeof(contactPin)/sizeof(int)); i++) { // sizeOf returns number of bytes
    pinMode(contactPin[i], OUTPUT);
    digitalWrite(contactPin[i], LOW);

    // Used to allow pullup resistors to settle
    delayMicroseconds(50);

//    Serial.print(i + 1);
//    Serial.print("\t");
    
    for (j = 0; j < (sizeof(contactPin)/sizeof(int)); j++) { 
      if ( i != j ) {
        contactVal = digitalRead(contactPin[j]);
//        Serial.print(contactVal);
        Serial.print(contactVal);
      }
      else {
//        Serial.print("1");
        Serial.print("1");
      }
//      Serial.print("  ");
    }
    pinMode(contactPin[i], INPUT_PULLUP);
//    Serial.println();
  }
  Serial.println();

  delay(100);
}
