#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <time.h>

//==========================
// WIFI
//==========================

const char* WIFI_SSID = "TI BROADCAST-2G";
const char* WIFI_PASSWORD = "guest-broadcast";

//==========================
// AWS IOT
//==========================

const char* AWS_ENDPOINT = "a3irogospm2wh4-ats.iot.us-east-1.amazonaws.com";
const char* MQTT_TOPIC = "senai/tanque1";

//==========================
// SENSOR
//==========================

#define TRIG_PIN 5
#define ECHO_PIN 18

//==========================
// TANQUE
//==========================

const float ALTURA_TANQUE = 30.0;
const float CAPACIDADE = 30.0;

//==========================
// OBJETOS
//==========================

WiFiClientSecure net;
MQTTClient client(1024);

//==========================
// VARIÁVEIS
//==========================

float distancia = 0;
float nivel = 0;
float percentual = 0;
float litros = 0;
float litrosAnterior = 0;
float taxaMudanca = 0;

unsigned long ultimoEnvio = 0;
const unsigned long intervalo = 5000;

//=====================================================
// CERTIFICADOS (mantidos iguais ao seu)
//=====================================================

static const char AMAZON_ROOT_CA[] PROGMEM = R"EOF(
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

static const char CERTIFICATE[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUejfJnjVs17AnvaW1hek9wlrRyjwwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MTAyOTIzNTUx
NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKrsIttuWWB1QkAaRbvC
9ty0hRMkPub5qhUwuS2GmKIJUVcwR0rZKvTbCtwTkYQxPa6m1uR/N8/CAqtpFAvM
/ThcliClWSCwP0UApA6CctSeeWMvGHeFRVILD02UjXT+Spb2LMkTTAfU6q6W95CK
mYryq1Zl7XFsC5NTKHZnKoBRyDjebVg+dG5S8sjDxcps/X79E1qBdn6/n02EbuSn
tnnCAwlqgwIWFYnkON/KXAlBMER7qrbQyYS3iPZcqY1h8AFg5IjO/8p6nVn0prk4
RqtzHWSWpoq20q24O1JMGalXLES7C61fK092Wj1n1fWVw4CL9umL5Rbhw2JrFoMJ
oy0CAwEAAaNgMF4wHwYDVR0jBBgwFoAUY4RM+hKnFsgDAXv/HnZGkdEoGrswHQYD
VR0OBBYEFNZoUA/1BA/rQT87jFFoIRRbMv/fMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCzIToDpL3b9YpfkRSqoFF9yWe6
CPy4TOQwOTCXTLT7H4XLK4eGmc0lh+kxQ3jxE+J2sR6NgfiyYYINEGUBq8tRkITP
Y4N5Id1ljBrf/vYymRFPr6/FQsTYK5RtKShqvWkMzM+JMyR28zR0hbIjAIz7yeAj
RYJjt56rBjMqrJe1LNfUX15xmlmjSnjl3wjioPD9Pg8momrTfX/ocASWeU/sobE5
3e6PddyUQZcFy8wTU3iHQG1Ef5iIKsI665igKmbf16OZAxNcx3/ncbH7I90jpfu6
U7i67xKQB7e0SCtd8LblRI+EEx9a46NB3qliZI8XxFUNd1v1K2I7NZBg1xV3
-----END CERTIFICATE-----
)EOF";

static const char PRIVATE_KEY[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEAquwi225ZYHVCQBpFu8L23LSFEyQ+5vmqFTC5LYaYoglRVzBH
Stkq9NsK3BORhDE9rqbW5H83z8ICq2kUC8z9OFyWIKVZILA/RQCkDoJy1J55Yy8Y
d4VFUgsPTZSNdP5KlvYsyRNMB9Tqrpb3kIqZivKrVmXtcWwLk1ModmcqgFHION5t
WD50blLyyMPFymz9fv0TWoF2fr+fTYRu5Ke2ecIDCWqDAhYVieQ438pcCUEwRHuq
ttDJhLeI9lypjWHwAWDkiM7/ynqdWfSmuThGq3MdZJamirbSrbg7UkwZqVcsRLsL
rV8rT3ZaPWfV9ZXDgIv26YvlFuHDYmsWgwmjLQIDAQABAoIBAAiHO74DJYaAk0ur
g+s4cSTWBQXNey3/KxNfmP3Ch38eQ+a7y/LaHyCUhVgxycL0ygUa4gIIrvo4zfVL
iuqD0easB/mdBnNxF9cTjxwCOkSNm7TtNqVQCwNq9dqDHQPHZGVf5yApTFPftc38
TmHx6HNcZFwtPcm5AGLbdVtjDmXUmdxHQDlxIyTYagnlTm+KLDr0/7RMoBBYF7yf
reMADPESvycaj06B0PwMgFpembcL0KRiU1s3GSHv++vcdn51KRSnydrRA3UtAtGn
KokEFkeLCNZ4TCmpcokeWwzBceMTfii7Squ2uJR4e3g+KOY5TbvMS5dGN1GSD8SH
ppbhL2kCgYEA43TRnJfZNvT+qBqQNBIBht3wApqnUujD+Ev3RBrV4En5a1TTzTFo
gNyX6TnfFSLgXXqfCGuBiFi+iM5YKstds1pp+LMh3M0VazyOBQdqKL3CfodYzdJN
dhsGF4FFQwmGadPzR3id16qWaT2P+ns11ZPPT3tmtu6UPtQJF4lWZhcCgYEAwF8g
+WgjHqFKW7kOb10+24gPNKj3Y9y+HLG+csJ3LzCXQPmBEhxfVUZP4YH9HrAVYPFm
SJzZe6ZTqyXztr1Yqr0hUCbe5/I95VdVtPBi6xIRqs1+xo+Tdv1pAZie+piBR4L8
bCHbc6YLUTx3eW6dn40RQJgfVzJfQzfXpxhoD1sCgYABTvW5ueHlAAnK+8QimIvr
s7EiwrL9thsSfZVhjen3DVXi+SzyvDsGqr4w+EVBQN8bio4JpVysZR5LeSxmqp7S
Xk3HwXgLZh6ebE3A16cTruvEgJDRNkl9yXACiJrWZOKiDzcSDS6TRAbzeggu/7By
lxQVN/eCS+MMbXnT6dVWZwKBgCOxguzD5cBJLQLX01XWd1NEa43ZzA07aArq7+tb
KcIXwfjMUqaJnSa1ZRzygYSwJ0dVgyT19WR7+ApJgG7nswv7C7NTgwiGafWjvsNT
1wbG04BdoKg2+6WYX2NsbhMROOuL4kot/gn3RSJNld1yKMzx38GceDQ1pHdUZS8b
2DMBAoGBAN15r69Kiwfl9Nqk/u3ZWvzslWORP2E7DFC6wLPgvuyz80u1VhcMp9Ze
MMNoVji3n8X/D0z9ea17KgdLzpag0xMXSrdKu8fgKg6Lx9YAiejyJ80y4KzvOCgZ
JMbhNaScx/sY4MTqvvDp39vk96oG9D2fSZRx7mrX/CTYt8Gyg7SU
-----END RSA PRIVATE KEY-----
)EOF";

//=====================================================
// WIFI
//=====================================================

void conectaWiFi()
{
    Serial.println("\nPreparando conexao Wi-Fi...");
    
    // Desliga qualquer tentativa de ligação presa em memória
    WiFi.disconnect(true); 
    delay(1000);

    Serial.println("Conectando ao Wi-Fi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int tentativas = 0;

    while (WiFi.status() != WL_CONNECTED && tentativas < 20)
    {
        delay(500);
        Serial.print(".");
        tentativas++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWi-Fi conectado!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nFalha no Wi-Fi");
    }
}

//=====================================================
// SINCRONIZAÇÃO DE HORA (CRÍTICO AWS)
//=====================================================

void sincronizaHora()
{
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print("Sincronizando hora");

    time_t now = time(nullptr);
    int tentativas = 0;

    while (now < 1700000000 && tentativas < 30)
    {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        tentativas++;
    }

    if (now < 1700000000)
        Serial.println("\nFalha ao sincronizar hora");
    else
        Serial.println("\nHora sincronizada!");
}

//=====================================================
// AWS CONNECT (SEGURA)
//=====================================================

void conectaAWS()
{
    net.setCACert(AMAZON_ROOT_CA);
    net.setCertificate(CERTIFICATE);
    net.setPrivateKey(PRIVATE_KEY);

    client.begin(AWS_ENDPOINT, 8883, net);
    client.setKeepAlive(60);
    client.setTimeout(5000);

    Serial.println("\nConectando AWS IoT...");

    int tentativas = 0;

    while (!client.connect("ESP32-Tanque-01") && tentativas < 10)
    {
        Serial.print(".");
        Serial.print(" erro=");
        Serial.println(client.lastError());
        delay(2000);
        tentativas++;
    }

    if (client.connected())
        Serial.println("\nAWS conectado!");
    else
        Serial.println("\nFALHA AWS IoT");
}

//=====================================================
// LEITURA HC-SR04
//=====================================================

float lerDistancia()
{
    float soma = 0;
    int validas = 0;

    for (int i = 0; i < 5; i++)
    {
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(5);

        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        long duracao = pulseIn(ECHO_PIN, HIGH, 30000);

        if (duracao > 0)
        {
            float d = duracao * 0.0343 / 2.0;
            soma += d;
            validas++;
        }

        delay(10);
    }

    if (validas == 0)
    {
        // mantém último valor válido implícito (evita zerar tudo)
        return distancia;
    }

    return soma / validas;
}
//=====================================================
// ENVIO AWS
//=====================================================

void enviarDados()
{
    float leitura = lerDistancia();

    // =========================
    // FILTRO FÍSICO DO SENSOR
    // =========================

    // evita valores absurdos
    if (leitura <= 0 || leitura > ALTURA_TANQUE + 10)
    {
        leitura = distancia; // mantém última válida
    }

    // sensor muito perto = tanque cheio (limite físico)
    if (leitura < 2.0)
        leitura = 2.0;

    if (leitura > ALTURA_TANQUE)
        leitura = ALTURA_TANQUE;

    distancia = leitura;

    // =========================
    // CÁLCULOS FÍSICOS
    // =========================

    nivel = ALTURA_TANQUE - distancia;

    if (nivel < 0)
        nivel = 0;

    if (nivel > ALTURA_TANQUE)
        nivel = ALTURA_TANQUE;

    percentual = (nivel / ALTURA_TANQUE) * 100.0;
    litros = (percentual / 100.0) * CAPACIDADE;

    // =========================
    // TAXA DE VARIAÇÃO REAL
    // =========================

    taxaMudanca = litros - litrosAnterior;

    litrosAnterior = litros;

    // =========================
    // BLOQUEIO DE DADOS INVÁLIDOS
    // =========================

    if (percentual == 0 && distancia == ALTURA_TANQUE)
    {
        Serial.println("Leitura inválida ignorada (tanque em estado neutro)");
        return;
    }

    // =========================
    // JSON
    // =========================

    JsonDocument doc;

    doc["distancia"] = round(distancia * 100) / 100.0;
    doc["nivel_cm"] = round(nivel * 100) / 100.0; 
    doc["nivel_reservatorio_percentual"] = round(percentual * 100) / 100.0;
    doc["nivel_reservatorio_litros"] = round(litros * 100) / 100.0;
    doc["consumo_tanque"] = round(taxaMudanca * 100) / 100.0;

    String json;
    serializeJson(doc, json);

    client.publish(MQTT_TOPIC, json);

    // =========================
    // DEBUG
    // =========================

    Serial.println("====================================");
    Serial.println("DADOS ENVIADOS");

    Serial.print("Distância: ");
    Serial.println(distancia);

    Serial.print("Nível: ");
    Serial.println(nivel);

    Serial.print("Percentual: ");
    Serial.println(percentual);

    Serial.print("Litros: ");
    Serial.println(litros);

    Serial.print("Taxa: ");
    Serial.println(taxaMudanca);

    Serial.println(json);
}

//=====================================================
// SETUP
//=====================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    conectaWiFi();
    sincronizaHora();
    conectaAWS();
}

//=====================================================
// LOOP
//=====================================================

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
        conectaWiFi();

    if (!client.connected())
        conectaAWS();

    client.loop();

    if (millis() - ultimoEnvio >= intervalo)
    {
        ultimoEnvio = millis();
        enviarDados();
    }
}