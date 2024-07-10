const int temperaturePin = A0;  // Analog pin where the TMP36 is connected

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read the analog value from the temperature sensor
  int sensorValue = analogRead(temperaturePin);

  // Convert the analog value to voltage
  float voltage = sensorValue * (5.0 / 1023.0);

  // Convert the voltage to temperature in degrees Celsius
  float temperatureCelsius = (voltage - 0.5) * 100.0;

  // Convert Celsius to Fahrenheit
  float temperatureFahrenheit = (temperatureCelsius * 9.0 / 5.0) + 32.0;

  // Print the temperature in both Celsius and Fahrenheit to the Serial Monitor
  Serial.print("Temperature: ");
//  Serial.print(temperatureCelsius);
//  Serial.print(" degrees Celsius | ");
  Serial.print(temperatureFahrenheit);
  Serial.println(" degrees Fahrenheit");

  // Categorize temperature into different ranges
  int temperatureCategory = -1;  // Default category for out-of-range values

  if (temperatureFahrenheit >= 95) {
    temperatureCategory = 0;
  } else {
    for (int i = 1; i <= 15; ++i) {
      float upperBound = 95 - (i - 1) * 5;
      float lowerBound = upperBound - 5;
      if (temperatureFahrenheit <= upperBound && temperatureFahrenheit >= lowerBound) {
        temperatureCategory = i;
        break;
      }
    }
  }

  // Print the temperature category
  //Serial.print("Temperature Category: ");
 // Serial.println(temperatureCategory);

  // Use the temperature category in your switch statement
  switch (temperatureCategory) {
    case 0:
      Serial.println("Category 0: Very Hot");
      break;
    case 1:
      Serial.println("Category 1: Hot");
      break;
    case 2:
      Serial.println("Category 2: Warm");
      break;
    case 3:
      Serial.println("Category 3: Mild");
      break;
    case 4:
      Serial.println("Category 4: Pleasant");
      break;
    case 5:
      Serial.println("Category 5: Indoors");
      break;
    case 6:
      Serial.println("Category 6: Also Indoors");
      break;
    case 7:
      Serial.println("Category 7: Getting co;der");
      break;
    case 8:
      Serial.println("Category 8: Getting colder");
      break;
    case 9:
      Serial.println("Category 9: Mild JK");
      break;
    case 10:
      Serial.println("Category 10: Gold Cold");
      break;
    case 11:
      Serial.println("Category 11: Extremely uncomfortable");
      break;
    case 12:
      Serial.println("Category 12: Go Indoors");
      break;  
    case 13:
      Serial.println("Category 13: Stupid cold");
      break;
    case 14:
      Serial.println("Category 14: Extremely Cold");
      break;   
    case 15:
      Serial.println("Category 15: Very Very Cold");
      break;
  //unnecesasary?    break; 
  }

  delay(1000);  // Delay for readability in the Serial Monitor
}
