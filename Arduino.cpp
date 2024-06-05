#include "thingProperties.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 7

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int valorHumedadSuelo = 0;
float humedadSuelo = 0.0;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

unsigned long lastActivationTime = 0; // Guardar la última vez que el relé fue activado
const unsigned long twoDays = 86400000; // Milisegundos en dos días
const unsigned long threeMinutes = 342000; // Milisegundos en tres minutos

const int HUMEDAD_SUELO_PIN = A0;

volatile int numPulsos2 = 0; // Contador de pulsos
int pinSensor2 = 2; // Pin del sensor
float factorConversion2 = 410.0; // 450 pulsos = 1 L
float caudal_L_m2, caudal_L_h2;
float consumoTotal_L2 = 0; // Variable para el consumo total acumulado
unsigned long ultimoRegistro2 = 0; // Tiempo del último registro
const int intervaloRegistro2 = 1000; // Intervalo de registro en milisegundos
int totalPulsos2 = 0; // Variable para guardar el total de pulsos contados

volatile int numPulsos1 = 0; // Contador de pulsos
int pinSensor1 = 3; // Pin del sensor
float factorConversion1 = 410.0; // 450 pulsos = 1 L
float caudal_L_m1, caudal_L_h1;
float consumoTotal_L1 = 0; // Variable para el consumo total acumulado
unsigned long ultimoRegistro1 = 0; // Tiempo del último registro
const int intervaloRegistro1 = 1000; // Intervalo de registro en milisegundos
int totalPulsos1 = 0; // Variable para guardar el total de pulsos contados


const int AirValue = 478;   // Valor máximo de humedad en aire
const int WaterValue = 192;  // Valor máximo de humedad en agua

void ContarPulsos2() {
  numPulsos2++; // Incrementa el contador de pulsos cada vez que se detecta un pulso
}
void ContarPulsos1() {
  numPulsos1++; // Incrementa el contador de pulsos cada vez que se detecta un pulso
}

const int numMuestras = 90;
float humedadSueloMuestras[numMuestras];
float temperaturaMuestras[numMuestras];
float humedadMuestras[numMuestras];
int indiceMuestra = 0;
unsigned long ultimoTiempo = 0;
unsigned long tiempoEspera = 240000;  // 4 minutos

const int RELAY1_PIN = 10;
const int RELAY2_PIN = 11;
const int RELAY3_PIN = 12;
const int RELAY4_PIN = 13;

void setup() {
  Serial.begin(9600);
  pinMode(pinSensor1, INPUT);
  pinMode(pinSensor2, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinSensor2), ContarPulsos2, RISING);
  attachInterrupt(digitalPinToInterrupt(pinSensor1), ContarPulsos1, RISING);
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  dht.begin();
  sensors.begin();
  pinMode(HUMEDAD_SUELO_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH); // Asegurarse de que el relé esté apagado al iniciar
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);
  Serial.println("Setup completo");
}

void loop() {
  unsigned long currentMillisForRelay3 = millis();
  Serial.println("Inicio de loop");
  int valorHumedadSuelo = analogRead(HUMEDAD_SUELO_PIN);
  float humedadSuelo = map(valorHumedadSuelo, AirValue, WaterValue, 0, 100);
  Serial.print("Humedad del suelo medida: ");
  Serial.println(humedadSuelo);
  Serial.print("Humedad del suelo enviada a la nube: ");
  Serial.println(humedad_Del_suelo);

  ArduinoCloud.update();
  if (millis() - ultimoRegistro2 >= intervaloRegistro2) {
    totalPulsos2 += numPulsos2; // Acumula el total de pulsos

    caudal_L_m2 = (numPulsos2 / factorConversion2) * 60; // Caudal por minuto
    caudal_L_h2 = caudal_L_m2 * 60; // Caudal por hora
    consumoTotal_L2 += (numPulsos2 / factorConversion2); // Suma los litros basados en el número de pulsos

    litrosdeagua2 = consumoTotal_L2; // Actualiza la variable de IoT Cloud
    //humedad_Del_suelo = medirHumedadSuelo(); // Actualiza la variable de humedad del suelo

    // Muestra los resultados en el monitor serial
    Serial.print("Caudal: ");
    Serial.print(caudal_L_m2, 3);
    Serial.print(" L/min, ");
    Serial.print(caudal_L_h2, 3);
    Serial.println(" L/h.");
    Serial.print("Consumo total: ");
    Serial.print(consumoTotal_L2, 3);
    Serial.println(" L totales.");
    Serial.print("Total de pulsos: ");
    Serial.println(totalPulsos2);
    

    numPulsos2 = 0; // Reinicia el contador de pulsos después de cada medición
    ultimoRegistro2 = millis(); // Actualiza el tiempo del último registro
  }
  if (millis() - ultimoRegistro1 >= intervaloRegistro1) {
    totalPulsos1 += numPulsos1; // Acumula el total de pulsos

    caudal_L_m1 = (numPulsos1 / factorConversion1) * 60; // Caudal por minuto
    caudal_L_h1 = caudal_L_m1 * 60; // Caudal por hora
    consumoTotal_L1 += (numPulsos1 / factorConversion1); // Suma los litros basados en el número de pulsos

    litrosdeagua1 = consumoTotal_L1; // Actualiza la variable de IoT Cloud
    //humedad_Del_suelo = medirHumedadSuelo(); // Actualiza la variable de humedad del suelo

    // Muestra los resultados en el monitor serial
    Serial.print("Caudal: ");
    Serial.print(caudal_L_m1, 3);
    Serial.print(" L/min, ");
    Serial.print(caudal_L_h1, 3);
    Serial.println(" L/h.");
    Serial.print("Consumo total: ");
    Serial.print(consumoTotal_L1, 3);
    Serial.println(" L totales.");
    Serial.print("Total de pulsos: ");
    Serial.println(totalPulsos1);
    
    numPulsos1 = 0; // Reinicia el contador de pulsos después de cada medición
    ultimoRegistro1 = millis(); // Actualiza el tiempo del último registro
  }
  unsigned long currentTime = millis();
  Serial.println("Después de ArduinoCloud.update()");
   // Control del relé 3 basado en el tiempo
  if (currentMillisForRelay3 - lastActivationTime >= twoDays) {
    digitalWrite(RELAY3_PIN, LOW); // Activar relé
    lastActivationTime = currentMillisForRelay3;
  }
  if (digitalRead(RELAY3_PIN) == LOW && currentMillisForRelay3 - lastActivationTime >= threeMinutes) {
    digitalWrite(RELAY3_PIN, HIGH); // Desactivar relé
  }


  if (currentTime - ultimoTiempo >= tiempoEspera) {
    Serial.println("Tiempo de espera alcanzado, calculando promedios y actualizando relés...");
    ultimoTiempo = currentTime;

    float averageHumidity = average(humedadMuestras, numMuestras);
    float averageTemperature = average(temperaturaMuestras, numMuestras);
    float averageSoilMoisture = average(humedadSueloMuestras, numMuestras);
    Serial.print("Promedio Humedad: "); Serial.println(averageHumidity);
    Serial.print("Promedio Temperatura: "); Serial.println(averageTemperature);
    Serial.print("Promedio Humedad Tierra: "); Serial.println(averageSoilMoisture);

    humedad = averageHumidity;
    temperatura = averageTemperature;
    humedad_Tierra = averageSoilMoisture;
    humedad_Del_suelo = humedadSuelo;
    temperaturaDeTierra = sensors.getTempCByIndex(0); // Actualizar con la última lectura válida

    Serial.print("Promedio Humedad: ");
    Serial.print(averageHumidity);
    Serial.print("%, Promedio Temperatura: ");
    Serial.print(averageTemperature);
    Serial.println("C");
    Serial.print("Promedio Humedad Tierra: ");
    Serial.print(averageSoilMoisture);
    

    // Activa los relés si es necesario
    if (humedadSuelo < 50) {
      digitalWrite(RELAY4_PIN, LOW); // Activar relé
    } else {
      digitalWrite(RELAY4_PIN, HIGH); // Desactivar relé
    }

    resetSamples(humedadMuestras, numMuestras);
    resetSamples(temperaturaMuestras, numMuestras);
    resetSamples(humedadSueloMuestras, numMuestras);
    indiceMuestra = 0;
  } else if (indiceMuestra < numMuestras) {
    collectData();
  }

  delay(2000); // Reduce la carga del loop
  Serial.println("Fin de loop");
}


void collectData() {
  Serial.println("Iniciando recolección de datos");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  sensors.requestTemperatures();
  float tierraTemp = sensors.getTempCByIndex(0);
  
  Serial.print("Valor crudo de humedad del suelo: ");
  Serial.println(valorHumedadSuelo);
  Serial.print("Humedad del suelo mapeada a porcentaje: ");
  Serial.println(humedadSuelo);

  if (!isnan(h) && !isnan(t) && !isnan(tierraTemp) && !isnan(humedadSuelo)) {
    humedadMuestras[indiceMuestra] = h;
    temperaturaMuestras[indiceMuestra] = t;
    humedadSueloMuestras[indiceMuestra] = humedadSuelo;
    indiceMuestra++;
  }
  Serial.println("Datos recolectados");
}


float average(float arr[], int numElements) {
  float sum = 0;
  for (int i = 0; i < numElements; i++) {
    sum += arr[i];
  }
  return sum / numElements;
}

void resetSamples(float arr[], int numElements) {
  for (int i = 0; i < numElements; i++) {
    arr[i] = 0;
  }
}


// Funciones de manejo de eventos de cambio en la nube IoT
void onHumedadChange() {
  //digitalWrite(RELAY1_PIN, humedad  ? LOW : HIGH);
}

void onTemperaturaChange() {
  //digitalWrite(RELAY2_PIN, temperatura  ? LOW : HIGH);
}

void onHumedadTierraChange() {
  digitalWrite(RELAY3_PIN, humedad_Tierra < 20 ? LOW : HIGH);
}

void onRelay1Change() {
  digitalWrite(RELAY1_PIN, relay1 ? LOW : HIGH);
}

void onRelay2Change() {
  digitalWrite(RELAY2_PIN, relay2 ? LOW : HIGH);
}

void onRelay3Change() {
  digitalWrite(RELAY3_PIN, relay3 ? LOW : HIGH);
}

void onRelay4Change() {
  digitalWrite(RELAY4_PIN, relay4 ? LOW : HIGH);
}

void onTemperaturaDeTierraChange() {
  Serial.print("Temperatura de Tierra ha cambiado a: ");
  Serial.println(temperaturaDeTierra);
}

void onVolumenTotalChange() {
  Serial.print("Volumen Total ha cambiado a: ");
  Serial.println(volumenTotal);
}

/*
  Since HumedadDelSuelo is READ_WRITE variable, onHumedadDelSueloChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onHumedadDelSueloChange()  {
  // Add your code here to act upon HumedadDelSuelo change
}
/*
  Since Litrosdeagua2 is READ_WRITE variable, onLitrosdeagua2Change() is
  executed every time a new value is received from IoT Cloud.
*/
void onLitrosdeagua2Change()  {
  // Add your code here to act upon Litrosdeagua2 change
}
/*
  Since Litrosdeagua1 is READ_WRITE variable, onLitrosdeagua1Change() is
  executed every time a new value is received from IoT Cloud.
*/
void onLitrosdeagua1Change()  {
  // Add your code here to act upon Litrosdeagua1 change
}