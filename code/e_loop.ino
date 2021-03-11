  //loop principal
 void loop()
{
    Serial.println("loop.");
    char fields_a_serem_enviados[200] = {0};
    float temperatura_lida = 0.0;
    float umidade_lida = 0.0;
    float solarlida=0.0;
    float temp_a_1=0.0;
    float temp_a_2=0.0;
    long RangeInCentimeters=0;
    
 
    /* Força desconexão ao ThingSpeak (se ainda estiver conectado) */
    if (client.connected())
    {
        client.stop();
        Serial.println("- Desconectado do ThingSpeak");
        Serial.println();
    }
 
    /* Garante que a conexão wi-fi esteja ativa */
    verifica_conexao_wifi();
    vazao=float(ler_vazao()); 
    /* Verifica se é o momento de enviar dados para o ThingSpeak */
    if( millis() - last_connection_time > INTERVALO_ENVIO_THINGSPEAK )
    {
        //temperatura_lida = 28 + temperatura_lida;
        //umidade_lida=5.5;
        solarlida=lersolar();
        temp_a_1=ler_temp_agua(1);
        temp_a_2=ler_temp_agua(2);
        
        //Serial.println(solarlida);
        RangeInCentimeters=ler_nivel();
        temperatura_lida = dht.readTemperature();
        umidade_lida = dht.readHumidity();
        float nivel=(float)RangeInCentimeters;
        sprintf(fields_a_serem_enviados,"field1=%.2f&field2=%.2f&field3=%.2f&field4=%.2f&field5=%.2f&field6=%.2f&field7=%.2f", temperatura_lida, umidade_lida,solarlida,vazao,temp_a_1,temp_a_2,RangeInCentimeters);
        envia_informacoes_thingspeak(fields_a_serem_enviados);
    }
    
    /* *****************
     *  Print informações
     *  ****************
     */
    temperatura_lida = dht.readTemperature();
    umidade_lida = dht.readHumidity();
    Serial.print("Temp ");
    Serial.println(temperatura_lida);
    Serial.print("Irra ");
    solarlida=lersolar();
    Serial.println(solarlida);
    Serial.print("Ta1 ");
    temp_a_1=ler_temp_agua(0);    
    Serial.print("Ta2 ");
    temp_a_2=ler_temp_agua(1);
    RangeInCentimeters= ler_nivel();
    Serial.print("Nivel ");
    Serial.println(RangeInCentimeters);
    
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    // print the number of seconds since reset:
    lcd.print("Irra ");
    lcd.print(solarlida);
    lcd.print("Niv ");
    lcd.print(RangeInCentimeters);
    
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    sensors.requestTemperatures(); 
    temp_a_1 = sensors.getTempCByIndex(0);
    lcd.print("T1 "); 
    lcd.print(temp_a_1); 
    sensors.requestTemperatures(); 
    temp_a_2 = sensors.getTempCByIndex(1);
    lcd.print("T2 "); 
    lcd.print(temp_a_2);  

        /****************************
 * Imprir dados no cartão SD
****************************/
  LerHora();
    // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print(dataoficial);
    dataFile.print(", Solar"); /*COLOCAR AQUI OS DADOS QUE QUEREM SER IMPRESSOS NO CARTÃO*/
    dataFile.print(solarlida);
    dataFile.print(", Temp ");
    dataFile.print(temperatura_lida);
    dataFile.print(", T1 "); 
    dataFile.print(temp_a_1);
    dataFile.print(", T2 "); 
    dataFile.print(temp_a_2);
    dataFile.print(", Niv ");
    dataFile.print(RangeInCentimeters);
    dataFile.println("*");
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataoficial);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
    delay(1000);
}

  
/********************
* Implementações
********************/
 
