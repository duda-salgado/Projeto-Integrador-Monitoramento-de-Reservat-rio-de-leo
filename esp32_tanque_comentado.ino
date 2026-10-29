//==================================================
// ESP32 + AWS IoT CORE + MQTT TLS
// Monitoramento de nivel e volume do tanque.
// Esta versao nao gera dados ficticios de temperatura ou pressao.
//==================================================

// Biblioteca para conectar o ESP32 a uma rede Wi-Fi.
#include <WiFi.h>
// Biblioteca para conexoes seguras TLS com o servidor.
#include <WiFiClientSecure.h>
// Biblioteca para publicar mensagens usando o protocolo MQTT.
#include <PubSubClient.h>
// Biblioteca para montar os dados em formato JSON.
#include <ArduinoJson.h>

//==================================================
// WIFI
//==================================================
// Por seguranca, preencha as credenciais no seu ambiente.
// Nao compartilhe senhas em codigo publico.
// Nome da rede Wi-Fi. Substitua pelo nome da sua rede.
const char* ssid = "SEU_SSID";
// Senha da rede Wi-Fi. Mantenha esse dado em segredo.
const char* password = "SUA_SENHA";

//==================================================
// AWS IoT
//==================================================
// Endpoint do AWS IoT Core fornecido pela sua conta.
const char* mqtt_server = "SEU_ENDPOINT_AWS_IOT";
// Porta padrao para MQTT com criptografia TLS.
const int mqtt_port = 8883;
// Topico MQTT em que os dados do tanque serao publicados.
const char* mqtt_topic = "senai/tanque1";

//==================================================
// CERTIFICADOS AWS
// Cole aqui o certificado raiz, o certificado do dispositivo
// e a chave privada correspondentes ao seu dispositivo AWS IoT.
// Nao compartilhe a chave privada.
//==================================================
// Certificado raiz usado para validar o servidor AWS.
static const char* amazon_root_ca = R"EOF(
COLE_AQUI_O_CERTIFICADO_RAIZ_DA_AWS
)EOF";

// Certificado digital que identifica o dispositivo perante a AWS.
static const char* certificate_pem_crt = R"KEY(
COLE_AQUI_O_CERTIFICADO_DO_DISPOSITIVO
)KEY";

// Chave privada correspondente ao certificado; nunca a compartilhe.
static const char* private_pem_key = R"KEY(
COLE_AQUI_A_CHAVE_PRIVADA_DO_DISPOSITIVO
)KEY";

//==================================================
// MQTT TLS
//==================================================
// Cliente de rede que permite conexoes protegidas por TLS.
WiFiClientSecure net;
// Cliente MQTT que utiliza a conexao TLS definida acima.
PubSubClient client(net);

//==================================================
// SENSOR E VARIAVEIS
//==================================================
// Pino que envia o pulso ultrassonico ao sensor HC-SR04.
#define TRIG 5
// Pino que recebe o eco retornado pelo sensor.
#define ECHO 18

// Ajuste para as dimensoes reais do tanque.
// Distancia de referencia em cm para o tanque vazio; ajuste ao tanque real.
const float altura_max = 50.0;  // cm: distancia do sensor ao fundo/limite de tanque vazio
// Capacidade util considerada para o tanque, em litros.
const float volume_max = 500.0; // litros: capacidade util considerada

// Armazena a distancia atual e a distancia da medicao anterior.
float leitura_atual = altura_max;
float leitura_anterior = altura_max;
// Volume anterior, usado para estimar a variacao de agua entre leituras.
float litros_anterior = 0.0;
// Taxa estimada de consumo em litros por hora.
float consumo_hora = 0.0;

//==================================================
// WIFI SETUP
//==================================================
// Conecta o ESP32 ao Wi-Fi e aguarda ate que a conexao seja estabelecida.
void setup_wifi() {
  Serial.println();
  Serial.print("Conectando ao Wi-Fi");

  // Inicia a conexao usando o nome e a senha configurados.
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

//==================================================
// AWS MQTT CONNECTION
//==================================================
// Configura TLS e conecta o ESP32 ao broker MQTT da AWS IoT Core.
void connectAWS() {
  // Define o certificado raiz para verificar a identidade do servidor.
  net.setCACert(amazon_root_ca);
  // Apresenta o certificado digital deste dispositivo.
  net.setCertificate(certificate_pem_crt);
  // Define a chave privada correspondente ao certificado do dispositivo.
  net.setPrivateKey(private_pem_key);
  client.setServer(mqtt_server, mqtt_port);

  while (!client.connected()) {
    Serial.print("Conectando a AWS IoT Core...");

    if (client.connect("PROJETO_INTEGRADOR")) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou. Estado MQTT: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

//==================================================
// SENSOR HC-SR04
//==================================================
// Dispara o sensor e calcula a distancia ate a superficie da agua.
float medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // Mede a duracao do eco; para de esperar apos 30.000 microssegundos.
  long duracao = pulseIn(ECHO, HIGH, 30000);

  // Sem eco, retorna -1 para sinalizar que a leitura nao e valida.
  if (duracao == 0) {
    return -1.0;
  }

  // Converte o tempo do eco em centimetros; divide por 2 pela ida e volta do som.
  float distancia = duracao * 0.0343 / 2.0;

  if (distancia < 2.0 || distancia > altura_max) {
    return -1.0;
  }

  return distancia;
}

//==================================================
// MEDIA FILTRADA: ignora leituras invalidas
//==================================================
// Faz cinco tentativas e calcula a media das leituras que forem validas.
float medirMedia() {
  float soma = 0.0;
  // Conta quantas das cinco tentativas produziram uma distancia valida.
  int leituras_validas = 0;

  for (int i = 0; i < 5; i++) {
    float distancia = medirDistancia();

    if (distancia >= 0.0) {
      soma += distancia;
      leituras_validas++;
    }

    delay(50);
  }

  if (leituras_validas == 0) {
    return -1.0;
  }

  return soma / leituras_validas;
}

//==================================================
// SETUP
//==================================================
// Executado uma vez quando o ESP32 e ligado ou reiniciado.
void setup() {
  // Inicializa o Monitor Serial para exibir mensagens de diagnostico.
  Serial.begin(115200);

  // TRIG envia o pulso (saida) e ECHO recebe o retorno (entrada).
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW);

  setup_wifi();
  connectAWS();

  // Nao inicializa o volume com uma leitura inventada.
  // Tenta obter uma primeira medicao real para inicializar o volume.
  float leitura_inicial = medirMedia();

  if (leitura_inicial >= 0.0) {
    leitura_atual = leitura_inicial;
    float nivel_inicial =
      ((altura_max - leitura_atual) / altura_max) * 100.0;
    nivel_inicial = constrain(nivel_inicial, 0.0, 100.0);
    litros_anterior = (nivel_inicial / 100.0) * volume_max;
  } else {
    Serial.println("Aviso: nao foi possivel obter leitura inicial valida.");
  }
}

//==================================================
// LOOP PRINCIPAL
//==================================================
// Executado repetidamente enquanto o ESP32 estiver ligado.
void loop() {
  // Se o Wi-Fi desconectar, tenta estabelecer a conexao novamente.
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  if (!client.connected()) {
    connectAWS();
  }
  // Mantem o cliente MQTT processando a comunicacao com a AWS.
  client.loop();

  // Faz uma nova media das leituras do sensor.
  float nova_leitura = medirMedia();

  // Se todas as leituras falharem, nao envia valores calculados
  // a partir de uma leitura antiga como se fossem atuais.
  // Se nao houver leitura valida, evita publicar valores desatualizados.
  if (nova_leitura < 0.0) {
    Serial.println("Erro: leitura do sensor invalida; envio ignorado.");
    // Aguarda cerca de 3 segundos antes de iniciar a proxima leitura.
  delay(3000);
    return;
  }

  leitura_anterior = leitura_atual;
  leitura_atual = constrain(nova_leitura, 0.0, altura_max);

  // Nivel do tanque em percentual
  // Converte distancia em percentual: quanto menor a distancia, mais cheio o tanque.
  float nivel_percentual =
    ((altura_max - leitura_atual) / altura_max) * 100.0;
  nivel_percentual = constrain(nivel_percentual, 0.0, 100.0);

  // Volume estimado em litros
  // Estima o volume em litros assumindo relacao linear entre nivel e capacidade.
  float litros = (nivel_percentual / 100.0) * volume_max;

  // Estimativa de consumo em L/h. Usa o tempo real decorrido
  // entre as atualizacoes, em vez de assumir exatamente 3 segundos.
  // Guarda o instante da medicao anterior para calcular o tempo decorrido.
  static unsigned long ultima_medicao_ms = 0;
  // millis() retorna o tempo desde que o ESP32 iniciou, em milissegundos.
  unsigned long agora_ms = millis();
  float intervalo_horas = 0.0;

  if (ultima_medicao_ms != 0) {
    unsigned long intervalo_ms = agora_ms - ultima_medicao_ms;
    if (intervalo_ms > 0) {
      intervalo_horas = (float)intervalo_ms / 3600000.0;
      // Diferenca positiva indica que o volume diminuiu entre as medicoes.
      float variacao_litros = litros_anterior - litros;

      // Se o volume aumentar, considera reabastecimento e zera o consumo estimado.
      if (variacao_litros > 0.0 && intervalo_horas > 0.0) {
        consumo_hora = variacao_litros / intervalo_horas;
      } else {
        consumo_hora = 0.0;
      }

      // Filtro simples para variacoes muito pequenas.
      if (consumo_hora < 1.0) {
        consumo_hora = 0.0;
      }
    }
  }

  ultima_medicao_ms = agora_ms;
  litros_anterior = litros;

  // O JSON contem apenas a distancia medida e valores calculados a partir dela.
  // Temperatura e pressao foram removidas porque nao ha sensores para medi-las.
  // Cria o objeto que vai organizar os campos da mensagem JSON.
  JsonDocument doc;
  // Distancia medida entre o sensor e a superficie da agua, em centimetros.
  doc["nivel_cm"] = leitura_atual;
  // Percentual de preenchimento calculado a partir da distancia.
  doc["nivel_percentual"] = nivel_percentual;
  // Volume estimado de agua no tanque, em litros.
  doc["litros"] = litros;
  // Taxa estimada de reducao do volume, em litros por hora.
  doc["consumo_hora"] = consumo_hora;

  // Espaco de memoria para armazenar o JSON convertido em texto.
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));

  // Publica no topico MQTT e guarda se a tentativa foi aceita pelo cliente.
  bool publicado = client.publish(mqtt_topic, buffer);

  Serial.println();
  Serial.println("========== DADOS DO TANQUE ==========");
  Serial.print("Nivel (cm): ");
  Serial.println(leitura_atual);
  Serial.print("Nivel (%): ");
  Serial.println(nivel_percentual);
  Serial.print("Volume estimado (L): ");
  Serial.println(litros);
  Serial.print("Consumo estimado (L/h): ");
  Serial.println(consumo_hora);
  Serial.print("Publicacao MQTT: ");
  Serial.println(publicado ? "sucesso" : "falha");
  Serial.println(buffer);
  Serial.println("=====================================");

  delay(3000);
}
