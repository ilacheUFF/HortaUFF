
#include <ESP8266WiFi.h> //essa biblioteca já vem com a IDE. Portanto, não é preciso baixar nenhuma biblioteca adicional
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
/*
* Defines do projeto
*/
//GPIO do NodeMCU que o pino de comunicação do sensor está ligado.
#define DHTPIN D8

 
/* defines - wi-fi */
#define SSID_REDE "LABMAQ_Estufa" /* coloque aqui o nome da rede que se deseja conectar */
#define SENHA_REDE "#trem015423" /* coloque aqui a senha da rede que se deseja conectar */
#define INTERVALO_ENVIO_THINGSPEAK 60000 /* intervalo entre envios de dados ao ThingSpeak (em ms) */
 
/* A biblioteca serve para os sensores DHT11, DHT22 e DHT21.
* No nosso caso, usaremos o DHT22, porém se você desejar utilizar
* algum dos outros disponíveis, basta descomentar a linha correspondente.
*/
 
#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321

#define FLOW_SENSOR  12  // Pin 12 = D6
#define FLOW_SENSOR_2  14  // Pin 12 = D6

/* CONSTANTES PARA O SENSOR DE FLUXO*/ 
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
float vazao=0.0;
 
/* constantes e variáveis globais PARA A CONEXÃO*/
char endereco_api_thingspeak[] = "api.thingspeak.com";
String chave_escrita_thingspeak = "GOZ2QKF7VL4ISFCZ";  /* Coloque aqui sua chave de escrita do seu canal */
unsigned long last_connection_time;

WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);

/* Define sensores de temperatura da agua
 *  
 */
// Data wire is plugged into digital pin d7 on the Arduino
#define ONE_WIRE_BUS D7

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

int deviceCount = 0;
float tempC;
 
/* prototypes */
void envia_informacoes_thingspeak(String string_dados);
void init_wifi(void);
void conecta_wifi(void);
void verifica_conexao_wifi(void);
void piscar(int tempo);
float lersolar(void);
float ler_vazao(void);
float ler_temp_agua(void);
 
/*
* Implementações
*/
 
/* Função: envia informações ao ThingSpeak
* Parâmetros: String com a informação a ser enviada
* Retorno: nenhum
*/
void envia_informacoes_thingspeak(String string_dados)
{
    if (client.connect(endereco_api_thingspeak, 80))
    {
        /* faz a requisição HTTP ao ThingSpeak */
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+chave_escrita_thingspeak+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(string_dados.length());
        client.print("\n\n");
        client.print(string_dados);
         
        last_connection_time = millis();
        Serial.println("- Informações enviadas ao ThingSpeak!");
        piscar(900);
    }
}
 
/* Função: inicializa wi-fi
* Parametros: nenhum
* Retorno: nenhum
*/
void init_wifi(void)
{
    Serial.println("------WI-FI -----");
    Serial.println("Conectando-se a rede: ");
    Serial.println(SSID_REDE);
    Serial.println("\nAguarde...");
 
    conecta_wifi();
}
 
/* Função: conecta-se a rede wi-fi
* Parametros: nenhum
* Retorno: nenhum
*/
void conecta_wifi(void)
{
    /* Se ja estiver conectado, nada é feito. */
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }
     
    /* refaz a conexão */
    WiFi.begin(SSID_REDE, SENHA_REDE);
     
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
    }
 
    Serial.println("Conectado com sucesso a rede wi-fi \n");
    Serial.println(SSID_REDE);
    piscar(500);
}
 
/* Função: verifica se a conexao wi-fi está ativa
* (e, em caso negativo, refaz a conexao)
* Parametros: nenhum
* Retorno: nenhum
*/
void verifica_conexao_wifi(void)
{
    conecta_wifi();
}

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
{
  pulseCount++;
}

float ler_temp_agua(int index)
{
   // Send command to all the sensors for temperature conversion
    sensors.requestTemperatures(); 
    Serial.print("Sensor ");
    tempC = sensors.getTempCByIndex(index);
    Serial.print(tempC); 
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

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    last_connection_time = 0;
 
    /* Inicializa sensor de temperatura e umidade relativa do ar */
    dht.begin();

    /* Inicio sensor de fluxo
     *  
     */     
  pinMode(FLOW_SENSOR, INPUT_PULLUP);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
 
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR), pulseCounter, FALLING);
  
  /* Setup sensor Temperatura de agua
   *  
   */
  sensors.begin();  // Start up the library
   // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  Serial.println("");
    /* Inicializa e conecta-se ao wi-fi */
    init_wifi();
}
 
//loop principal
void loop()
{
    char fields_a_serem_enviados[200] = {0};
    float temperatura_lida = 0.0;
    float umidade_lida = 0.0;
    float solarlida=0.0;
    float temp_a_1=0.0;
    float temp_a_2=0.0;
   
 
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
        
        Serial.println(solarlida);
        
        temperatura_lida = dht.readTemperature();
        umidade_lida = dht.readHumidity();
        sprintf(fields_a_serem_enviados,"field1=%.2f&field2=%.2f&field3=%.2f&field4=%.2f&field5=%.2f&field6=%.2f", temperatura_lida, umidade_lida,solarlida,vazao,temp_a_1,temp_a_2);
        envia_informacoes_thingspeak(fields_a_serem_enviados);
    }
    temperatura_lida = dht.readTemperature();
    umidade_lida = dht.readHumidity();
    Serial.print("Temperatura");
    Serial.println(temperatura_lida);
 
    delay(50);
}
