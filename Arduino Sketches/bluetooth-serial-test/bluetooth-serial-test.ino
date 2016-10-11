/*
  Modified version of Jim Lindblom's
  Example Bluetooth Serial Passthrough Sketch
  Tests serial communication set up at 9600 baud

 SparkFun Electronics
 original date: February 26, 2013
 license: Public domain

 This example sketch converts an RN-42 Bluetooth module to
 communicate at 9600 bps (from 115200), and pass any serial
 data between Serial Monitor and bluetooth module.
 */

#define bluetooth Serial1
// For Teensy LC board (hint):
// TX-O pin of Bluetooth Mate: RX pin 0 of Teensy
// RX-I pin of Bluetooth Mate: TX pin 1 of Teensy

// LED pin on Teensy 3.0
//int led = 13;

void setup()
{
  Serial.begin(115200);  // Begin the serial monitor at 9600bps
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
}

void loop()
{

}
