
#include <ESP8266WiFi.h> //essa biblioteca já vem com a IDE. Portanto, não é preciso baixar nenhuma biblioteca adicional
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Ultrasonic.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include <RtcDS3231.h>
#include <SPI.h>
#include <SD.h>

/*******************************
 * DEFINIÇÕES PARA O DATALOGGER
 ******************************/
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */

/*Variaveis para datalogger*/
char dataoficial[20];
const int chipSelect = D8;



 
/*
* Defines do projeto
*/
//GPIO do NodeMCU que o pino de comunicação do sensor está ligado.


 
/****************
* defines - wi-fi 
*****************/
#define SSID_REDE "LABMAQ_Estufa" /* coloque aqui o nome da rede que se deseja conectar */
#define SENHA_REDE "#trem015423" /* coloque aqui a senha da rede que se deseja conectar */
#define INTERVALO_ENVIO_THINGSPEAK 120000 /* intervalo entre envios de dados ao ThingSpeak (em ms) */
 
/*************************
 * DHT22
 *************************/
#define DHTPIN 10 
#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321

/*************************
 * SENSORES DE VAZÃO
 *************************/
#define FLOW_SENSOR  9  // Pin 12 = D6
#define FLOW_SENSOR_2  8  // Pin 12 = D6

/*********************************
VARIAVIES 
**********************************/ 


/*PARA O SENSOR DE FLUXO*/
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

/* *********************************************
 *  Define sensores de temperatura da agua
 * ******************************************/
// Data wire is plugged into digital pin d4 on the Nodemcu
#define ONE_WIRE_BUS D4
// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

/* *****************
 *  ULTRASOM
 ******************/

Ultrasonic ultrasonic(D3);
int deviceCount = 0;
float tempC;

/************
 * LCD
 ************/
 
rgb_lcd lcd;  //Cria um item chamado LCD

 
/* prototypes */
void envia_informacoes_thingspeak(String string_dados);
void init_wifi(void);
void conecta_wifi(void);
void verifica_conexao_wifi(void);
void piscar(int tempo);
float lersolar(void);
float ler_vazao(void);
float ler_temp_agua(void);
long ler_nivel(void);
void pulseCounter(void);

void setup()
{
    Serial.begin(115200);
    delay(50);
    Serial.println("Inicio..");
    pinMode(LED_BUILTIN, OUTPUT);
    last_connection_time = 0;
 
    /* Inicio sensor de fluxo  */     
    pinMode(FLOW_SENSOR, INPUT_PULLUP);
    pulseCount = 0;
    flowRate = 0.0;
    flowMilliLitres = 0;
    totalMilliLitres = 0;
    previousMillis = 0;
    Serial.println("Inicio2..");
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR), pulseCounter, FALLING);
  /* Inicializa sensor de temperatura e umidade relativa do ar */
    dht.begin();
  /* Inicioa relogio e SD */     
  InitRelogioSD();
  /* Setup sensor Temperatura de agua*/ 
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

  /* ***********
   *  LCD
   *  **********
   */

   lcd.begin(16, 2);
   lcd.print("Ligando o trem..");
   delay(1000);
}
