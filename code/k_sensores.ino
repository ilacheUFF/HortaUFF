
/* Função para piscar o led cada vez que a informação é enviada
 *  
 *  
 */
 void piscar(int tempo)
 {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(tempo);
  digitalWrite(LED_BUILTIN, LOW);
  delay(tempo);
 }
  /*  
   *   sensor irrigação solar
   */

float lersolar(void)
{
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  return(voltage);
}


/*SENSOR DE FLUXO
 * 
 */
 void IRAM_ATTR pulseCounter()
 //void pulseCounter()
{
  pulseCount++;
}

float ler_temp_agua(int index)
{
   // Send command to all the sensors for temperature conversion
    sensors.requestTemperatures(); 
    //Serial.print("Sensor ");
    //sensors.getTempC(deviceAddress);
    tempC = sensors.getTempCByIndex(index);
    Serial.println(tempC); 
    return(tempC);
}

float ler_vazao(void)
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;
 
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
 
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);
 
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(float(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space
 
 
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");
  }
  return(flowRate);
}
/****/
long ler_nivel(void)
{  
  long RangeInCentimeters;
  RangeInCentimeters = ultrasonic.MeasureInCentimeters(); // two measurements should keep an interval
  return(RangeInCentimeters);  
}
