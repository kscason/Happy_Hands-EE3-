int flexSensorPin = A9; //analog pin 0

void setup(){
  Serial.begin(9600);
  pinMode(flexSensorPin, INPUT_PULLUP);
}

void loop(){
  int flexSensorReading = analogRead(flexSensorPin); 

  Serial.println(flexSensorReading);

  //In my tests I was getting a reading on the arduino between 512, and 614. 
  //Using map(), you can convert that to a larger range like 0-100.
  
//  int flex0to100 = map(flexSensorReading, 330, 499, 0, 100);
//  Serial.println(flex0to100);

  delay(250); //just here to slow down the output for easier reading
}
