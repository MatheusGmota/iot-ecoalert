#define THINGER_SERIAL_DEBUG
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ThingerESP32.h>
#include <NewPing.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

// === Definições para o Sensor DHT22 === 
#define DHTPIN 4      
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE); // Cria um objeto DHT

// === Definições para o Sensor Ultrassônico (HC-SR04) === 
#define TRIGGER_PIN 5
#define ECHO_PIN 18
#define MAX_DISTANCE 200

// Cria um objeto NewPing para o sensor ultrassônico
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// === Definições para buzzer e led === 
#define LED1 33
#define BUZZERPIN 14

// === Definições para o Cálculo do Nível de Água === 
const float ALTURA_SENSOR_ACIMA_DA_AGUA_VAZIA = 150; 
const float PROFUNDIDADE_TOTAL_RECIPIENTE = 100;

// === Definições para a API Java === 
const char* JAVA_API_URL = API_URL;

// === Configuração do Thinger.io === 
ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED1, OUTPUT);
  pinMode(BUZZERPIN, OUTPUT);

  ledcSetup(0, 2000, 8);
  ledcAttachPin(BUZZERPIN, 0);

  // Configura a conexão Wi-Fi
  thing.add_wifi(SSID, SSID_PASSWORD);
}

void loop() {
  thing.handle(); // Mantém a conexão com o Thinger.io e processa eventos

  // === Leitura do Sensor DHT22 ===
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Erro na captura da leitura do DHT22");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temp);
    Serial.print(" °C, Umidade: ");
    Serial.print(hum);
    Serial.println(" %");
  }

  // === Leitura do Sensor Ultrassônico ===
  delay(50); // Pequeno atraso para o sensor ultrassônico
  unsigned int uS = sonar.ping(); // Envia um pulso e retorna o tempo em microssegundos
  float distanciaCm = sonar.convert_cm(uS); // Converte o tempo para distância em centímetros

  // Se o sensor não detectou nada (distância 0 ou MAX_DISTANCE), pode significar erro ou fora do alcance
  if (distanciaCm == 0 || distanciaCm > MAX_DISTANCE) {
    Serial.println("Erro ou fora de alcance na leitura do sensor ultrassônico.");
    distanciaCm = -1;
  } else {
    Serial.print("Distancia Ultrassonica: ");
    Serial.print(distanciaCm);
    Serial.println(" cm");
  }

  // === Cálculo do Nível de Água ===
  float nivelAguaCm = -1; // Valor padrão para indicar erro ou não aplicável
  float porcentagemNivel = -1;

  if (distanciaCm != -1) { // Se a leitura da distância foi válida
    // nivelAguaCm é a altura da coluna de água
    nivelAguaCm = ALTURA_SENSOR_ACIMA_DA_AGUA_VAZIA - distanciaCm;

    // Garante que o nível não seja negativo ou maior que a profundidade total
    if (nivelAguaCm < 0) nivelAguaCm = 0;
    if (nivelAguaCm > PROFUNDIDADE_TOTAL_RECIPIENTE) nivelAguaCm = PROFUNDIDADE_TOTAL_RECIPIENTE;

    porcentagemNivel = (nivelAguaCm / PROFUNDIDADE_TOTAL_RECIPIENTE) * 100.0;

    Serial.print("Nivel da agua (cm): ");
    Serial.print(nivelAguaCm);
    Serial.print(" Porcentagem: ");
    Serial.print(porcentagemNivel);
    Serial.println("%");
  }

  // === Envio de Dados para o Thinger.io ===
  pson data;
  if (!isnan(temp)) data["temperature"] = temp;
  if (!isnan(hum)) data["humidity"] = hum;
  if (distanciaCm != -1) {
    data["distance_cm"] = distanciaCm;
    data["water_level_cm"] = nivelAguaCm;
    data["water_level_pct"] = porcentagemNivel;
  }
  data["location"] = "Rio Tietê";

  // Condição de temperatura e umidade anormal
  if (temp > 39 || temp < -40 || hum > 70 || hum < 20) {
    ledcWrite(0, 128);
    digitalWrite(LED1, HIGH);
    delay(300);
    ledcWrite(0,0);
    digitalWrite(LED1, LOW);
    delay(300);
  } else {
    ledcWrite(0,0);
    digitalWrite(LED1, LOW);
  }

  thing.write_bucket("meu_bucket", data); // Envia os dados para o bucket no Thinger.io

  JsonDocument jsonDoc;
  jsonDoc["estado"] = "São Paulo";
  jsonDoc["cidade"] = "Osasco";
  jsonDoc["latitude"] = "-23.5505";
  jsonDoc["longitude"] = "-46.6333";
  jsonDoc["temperatura"] = temp;
  jsonDoc["umidade"] = hum;
  jsonDoc["nivelAguaCm"] = nivelAguaCm;
  jsonDoc["porcentagemNivel"] = porcentagemNivel;

  HTTPClient http;
  http.begin(JAVA_API_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Accept", "application/json");

  String jsonBody;
  serializeJson(jsonDoc, jsonBody);

  int httpResponseCode = http.POST(jsonBody);

  if (httpResponseCode > 0) { // Verifica se a requisição foi bem-sucedida
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString(); // Obtém a resposta da API
    Serial.println(response);
  } else {
    Serial.print("Erro na requisição HTTP. Código: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  delay(60000); // Espera 1 minuto antes da próxima leitura/envio
}