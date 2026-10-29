//==================================================
// ESP32 + AWS IOT CORE + MQTT TLS (100% CORRIGIDO)
//==================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//==================================================
// WIFI
//==================================================
const char* ssid = "roteador_duda";
const char* password = "duda0102";

//==================================================
// AWS IOT
//==================================================
const char* mqtt_server = "a2zq6tatejiz4g-ats.iot.us-east-1.amazonaws.com";
const int mqtt_port = 8883;
const char* mqtt_topic = "senai/tanque1";

//==================================================
// CERTIFICADOS AWS
//==================================================
static const char* amazon_root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char* certificate_pem_crt = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUaSX5RExr+cPM45gQFL1UafFz/oowDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDUyMjIzMzU0
NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALLb052lK+7qwkWMnuIN
cJnRdr3HugXssdVbpXVi2EcM0azqZYP6oKyHvzAxIkfBYYPKRHCj+rfQToAxuQZ8
p0kd8bhfdCsZysOTTPUWy0jKLw8zgbSstaM8hVf0paiV3rPz7GG+1vfQPRzW/CIH
6hfMCNlEywOnv1I1OcmOfZCjhN3FZxnCUyT1YborRCGriTnaVn1kFy2BAjrtxOAB
4kNoU4tfKfdmM3dg+lUc/GhAIUfsEbzKqybeS3JHNzM2hheNR52hfco2YKvXKG89
UPUhuUjTVtosAoLCfwIop+m4Vfis+miM24vFbnm0cAwB+u6GyhK8wPTpT3+5P+Qp
sFMCAwEAAaNgMF4wHwYDVR0jBBgwFoAUTDi+IGb68p0o4ELMYbvd+8tgkWcwHQYD
VR0OBBYEFMR12THNvwxl9arGR5z+D0BfSOzAMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAzVUzXc3MV+Ri6bFY2zs+O9NqD
gPzgj9W3tLF1nbS8hqKEIJom/PqrDAw6kq3+PwJcyJ+TGB8vwgzpkMfu74r31Oj5
6y/KSB+N48CqjuUZZG7dbAz4nJkQMXr+BA6FRQXPc3r5BXu4HLPiu3C+5S+G3kvI
GWxnrOLrBKXdgFloA3d1EwkSizyjhmGg9V2t/YdfDCw+EM9IQCrxQ6l83/NrAY6Q
qxKkSt5ZsmRxxXRpvsHuhMPBZr8lVpaLdKM5iT60tC2yPQp0Brng4Hu67ejGBtR1
EoISSbJAHNmX4dE/HhXbD20U8a7tWkCAU4XqW+Ug96ruytMS8e5E758GLofG
-----END CERTIFICATE-----
)KEY";

static const char* private_pem_key = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEpQIBAAKCAQEAstvTnaUr7urCRYye4g1wmdF2vce6Beyx1VuldWLYRwzRrOpl
g/qgrIe/MDEiR8Fhg8pEcKP6t9BOgDG5BnynSR3xuF90KxnKw5NM9RbLSMovDzOB
tKy1ozyFV/SlqJXes/PsYb7W99A9HNb8IgfqF8wI2UTLA6e/UjU5yY59kKOE3cVn
GcJTJPVhuitEIauJOdpWfWQXLYECOu3E4AHiQ2hTi18p92Yzd2D6VRz8aEAhR+wR
vMqrJt5Lckc3MzaGF41HnaF9yjZgq9cobz1Q9SG5SNNW2iwCgsJ/Aiin6bhV+Kz6
aIzbi8VuebRwDAH67obKErzA9OlPf7k/5CmwUwIDAQABAoIBAQCFG44oY1UH0FRW
OCnCJ6hmAPnykOJDKHzMQREpBC/a6JJesm5sQygyXjAN6HMPnb45u10lyxN6mp7h
Tp/ZUQmaI5vRz8//h4lb9ZKvT5BoEnflQzrDQElJO3AtHhfvF+IOYHjJ4aHP+XyR
qvknRmaT4Tewoj7H8J40ywG/m7ic26YSLoWtxBSw2OHP8Wbneqswp421fFoyekMa
sSvfUtM+UA5ESI9MYnbEjhRcxxwPjnjt/wN9HV2wXTDPr02alc0jdKwzp3NvQZtn
R9P4Va+VhHPKoeMW2Awed04trX7aW53asTRvmv4+QecXWX7m1WaL1lb3UYezQy9l
EbCsiyRJAoGBANxokF4HhvDP0BB/m28Ldo/R/TUXFQaFQIFNCBKyIKSaojaL03/9
Qn9HrwmK+Xz9lt8NFaCQ9thaOqwOqf/OoHy6aMW4s02+JdqNZjUhrS5Ap0YkWDMw
NfTkNHd/zWJBBpASsixMECr054i9ODH1Tb9ByZOMZHwVWAJzeM7dB/h/AoGBAM+9
pJATATruITQ64tqBDyHQiBUIgyqXrIAAH/nNRERghGqK1Cr84BaDGBqQywDyy/C1
SufAZUjct4UZN4pNuwwflJI6EWAN4rrYAEMxtvTG4WSaVYnP9N9Wg0Fh873rnT5s
0OMyXKGCGLQErJ/suPXVOUewry8U0+utCbasS/4tAoGAItJaTKC/zvfmFVUzx112
Q86XRHX7GmafBE6ZQermgVr02c1JguUORnGa09UjCX4/Acpdalq8xP2sW1aNwR5x
+IKy3toJafTqohbI2QgTBkWsp8ZCMQQi4ifTqjWEy0sesFBj2d/ChibfFZlfcXUh
7V+Fa0w46+mrV+fKZFRDWB0CgYEAk+Af0zrkBknaMu6z6uwxlFV3eNltXRVIgnkU
SeI29SFrQNf//XWyIfRBZS+qgctTJs42Os/2iS4sKL9xjK6WNiQIwyo4khJwUaZO
UYaDAKzNZqqBty7j1VMk7ZbHRWsBrJ0S+Jcx3vnXkdU9+D0u0Ddupbx8h1whGkM7
heAR3CUCgYEAj6m//nunplC7vAd73RhoFIse7guybSkVgeYTUnIMV1HV4v+TvXtd
DmUN/8mu95n6aVEjBQfWWLC1BKPJGtkgawFL5rODu05YXbOrwbBUVWU9VUNj28lf
M8k3GfsE44f+3LRJfeaJ3kI6lsQc94UII+7WXNByuOb4xL2MjtxDf78=
-----END RSA PRIVATE KEY-----
)KEY";

//==================================================
// MQTT TLS
//==================================================
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

//==================================================
// SENSOR E VARIÁVEIS
//==================================================
#define TRIG 5
#define ECHO 18

float altura_max = 50.0;
float volume_max = 500.0;

// Inicializadas preventivamente com a altura máxima (tanque vazio por padrão)
float leitura_atual = altura_max; 
float leitura_anterior = altura_max;

float consumo_hora = 0;
float temperatura = 0;
float pressao = 0;
float litros_anterior = 0;

//==================================================
// WIFI SETUP
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
  Serial.println("WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

//==================================================
// AWS MQTT CONNECTION
//==================================================
void connectAWS() {
  net.setCACert(amazon_root_ca);
  net.setCertificate(certificate_pem_crt);
  net.setPrivateKey(private_pem_key);

  client.setServer(mqtt_server, mqtt_port);

  while (!client.connected()) {
    Serial.print("Conectando AWS IoT...");

    if (client.connect("PROJETO_INTEGRADOR")) {
      Serial.println("CONECTADO!");
    } else {
      Serial.print("Falhou: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

//==================================================
// SENSOR HC-SR04
//==================================================
float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH, 30000);

  if (duracao == 0) {
    return leitura_anterior;
  }

  float distancia = duracao * 0.034 / 2;

  if (distancia < 2 || distancia > altura_max) {
    return leitura_anterior;
  }

  return distancia;
}

//==================================================
// MÉDIA FILTRADA (CORRIGIDO)
//==================================================
float medirMedia() {
  float soma = 0;

  for (int i = 0; i < 5; i++) {
    soma += medirDistancia(); // <--- Linha corrigida de 'som' para 'soma'
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
  connectAWS();
  randomSeed(micros());

  // Garante uma leitura inicial estável para evitar picos no primeiro loop
  leitura_atual = medirMedia();
  float nivel_inicial = ((altura_max - leitura_atual) / altura_max) * 100.0;
  litros_anterior = (nivel_inicial / 100.0) * volume_max;
}

//==================================================
// LOOP PRINCIPAL
//==================================================
void loop() {
  if (!client.connected()) {
    connectAWS();
  }
  client.loop();

  leitura_anterior = leitura_atual;
  leitura_atual = medirMedia();
  leitura_atual = constrain(leitura_atual, 0, altura_max);

  //========================================
  // NÍVEL DO TANQUE
  //========================================
  float nivel_percentual = ((altura_max - leitura_atual) / altura_max) * 100.0;

  //========================================
  // VOLUME EM LITROS
  //========================================
  float litros = (nivel_percentual / 100.0) * volume_max;

  //========================================
  // CONSUMO DO TANQUE (L/h) - LOGICA SUAVE
  //========================================
  float consumo = litros_anterior - litros;

  // Ignora se o tanque estiver enchendo/reabastecendo
  if (consumo < 0) {
    consumo = 0;
  }

  // Intervalo de 3s (3600s / 3s = fator de 1200)
  consumo_hora = consumo * 1200.0;

  // Trava para eliminar micro ruídos/oscilações milimétricas do sensor ultrassônico
  if (consumo_hora < 1.0) {
    consumo_hora = 0;
  }

  litros_anterior = litros;

  //========================================
  // VARIÁVEIS DE SIMULAÇÃO (TEMPERATURA E PRESSÃO)
  //========================================
  temperatura = random(40, 81);
  pressao = random(1200, 1801);

  //========================================
  // ARDUINO JSON (COMPATÍVEL COM V6 E V7)
  //========================================
  JsonDocument doc; 

  doc["nivel_cm"] = leitura_atual;
  doc["nivel_percentual"] = nivel_percentual;
  doc["litros"] = litros;
  doc["consumo_hora"] = consumo_hora;
  doc["temperatura"] = temperatura;
  doc["pressao"] = pressao;

  char buffer[256];
  serializeJson(doc, buffer);

  //========================================
  // ENVIA PARA AWS IOT CORE
  //========================================
  client.publish(mqtt_topic, buffer);

  //========================================
  // SERIAL MONITOR
  //========================================
  Serial.println();
  Serial.println("========== DADOS ENVIADOS AWS ==========");
  Serial.print("Nivel (cm): ");      Serial.println(leitura_atual);
  Serial.print("Nivel (%): ");       Serial.println(nivel_percentual);
  Serial.print("Volume (L): ");      Serial.println(litros);
  Serial.print("Consumo (L/h): ");   Serial.println(consumo_hora);
  Serial.print("Temperatura (C): "); Serial.println(temperatura);
  Serial.print("Pressao (bar): ");   Serial.println(pressao);
  Serial.println("========================================");
  Serial.println(buffer);

  delay(3000);
}
