unsigned pin[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}; // Pick digital pins for contacts
unsigned i, j;

void setup() {
  Serial.begin(9600);
  for (i = 0; i < (sizeof(pin)/sizeof(int)); i++) {  // Initialize all pins to INPUT
    pinMode(pin[i], INPUT_PULLUP);
  }
}

void loop() {
  // Write one pin as high and read from the others
  for (i = 0; i < (sizeof(pin)/sizeof(int)); i++) { // sizeOf returns number of bytes
    pinMode(pin[i], OUTPUT);
    digitalWrite(pin[i], LOW);
    for (j = 0; j < (sizeof(pin)/sizeof(int)); j++) { 
      if ( i != j )
        Serial.print(digitalRead(pin[j]));
      else
        Serial.print("1");
      Serial.print("  ");
    }
    pinMode(pin[i], INPUT_PULLUP);
    Serial.println();
  }
  Serial.println();
  delay(500); //just here to slow down the output for easier reading
}
