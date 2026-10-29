//==================================================
// ESP32 + HC-SR04 + MQTT + JSON
// COLETA DE DADOS INDUSTRIAIS
//==================================================

//==================================================
// BIBLIOTECAS
//==================================================

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//==================================================
// WIFI
//==================================================

const char* ssid = "roteador_duda";
const char* password = "duda0102";

//==================================================
// MQTT
//==================================================

const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

//==================================================
// SENSOR HC-SR04
//==================================================

#define TRIG 5
#define ECHO 18

//==================================================
// CONFIGURAÇÕES DO TANQUE
//==================================================

float altura_max = 50.0;
float volume_max = 500.0;

//==================================================
// VARIÁVEIS
//==================================================

float leitura_atual = 0;
float leitura_anterior = 0;

float taxa_mudanca = 0;

float temperatura = 0;
float pressao = 0;

//==================================================
// WIFI
//==================================================

void setup_wifi() {

  Serial.println();
  Serial.print("Conectando WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

//==================================================
// MQTT
//==================================================

void reconnect() {

  while (!client.connected()) {

    Serial.print("Conectando MQTT...");

    if (client.connect("ESP32_TANQUE")) {

      Serial.println("OK");

    } else {

      Serial.print("Erro: ");
      Serial.println(client.state());

      delay(2000);
    }
  }
}

//==================================================
// LEITURA HC-SR04
//==================================================

float medirDistancia() {

  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH, 30000);

  // Sem eco
  if (duracao == 0) {

    return leitura_anterior;
  }

  float distancia = duracao * 0.034 / 2;

  // Limites válidos
  if (distancia < 2 || distancia > altura_max) {

    return leitura_anterior;
  }

  return distancia;
}

//==================================================
// MÉDIA DE LEITURAS
//==================================================

float medirMedia() {

  float soma = 0;

  for (int i = 0; i < 5; i++) {

    soma += medirDistancia();

    delay(50);
  }

  return soma / 5.0;
}

//==================================================
// SETUP
//==================================================

void setup() {

  Serial.begin(115200);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  setup_wifi();

  client.setServer(mqtt_server, 1883);

  randomSeed(micros());
}

//==================================================
// LOOP
//==================================================

void loop() {

  // MQTT
  if (!client.connected()) {

    reconnect();
  }

  client.loop();

  // Guarda leitura anterior
  leitura_anterior = leitura_atual;

  // Nova leitura
  leitura_atual = medirMedia();

  leitura_atual =
      constrain(leitura_atual, 0, altura_max);

  //================================================
  // NÍVEL %
  //================================================

  float nivel_percentual =
      ((altura_max - leitura_atual) / altura_max) * 100.0;

  //================================================
  // LITROS
  //================================================

  float litros =
      (nivel_percentual / 100.0) * volume_max;

  //================================================
  // TAXA DE MUDANÇA
  //================================================

  taxa_mudanca =
      leitura_atual - leitura_anterior;

  //================================================
  // DADOS SIMULADOS
  //================================================

  temperatura = random(20, 90);

  pressao = random(1000, 2000);

  //================================================
  // JSON
  //================================================

  StaticJsonDocument<256> doc;

  doc["nivel_cm"] = leitura_atual;
  doc["nivel_percentual"] = nivel_percentual;
  doc["litros"] = litros;
  doc["taxa_mudanca"] = taxa_mudanca;
  doc["temperatura"] = temperatura;
  doc["pressao"] = pressao;

  char buffer[256];

  serializeJson(doc, buffer);

  //================================================
  // MQTT
  //================================================

  client.publish("senai/tanque1", buffer);

  //================================================
  // SERIAL
  //================================================

  Serial.println(buffer);

  delay(2000);
}