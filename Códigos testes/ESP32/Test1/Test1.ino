#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// =====================================================
// CONFIGURAÇÃO DO WI-FI
// =====================================================

const char* ssid = "Monica";
const char* password = "familia123";

// =====================================================
// CONFIGURAÇÃO DE DATA E HORA
// =====================================================

// Fuso horário de Brasília (UTC-3)
const long GMT_OFFSET_SEC = -3 * 3600;

// Horário de verão
const int DAYLIGHT_OFFSET_SEC = 0;

// Servidor NTP
const char* ntpServer = "pool.ntp.org";

// =====================================================
// SERVIDOR WEB
// =====================================================

WebServer server(80);

// =====================================================
// VARIÁVEIS
// =====================================================

int idPessoa = -1;
String nomePessoa = "Desconhecido";

String dataAtual = "";
String horaAtual = "";

// =====================================================
// CONFIGURAÇÃO CORS
// =====================================================

void adicionarCORS() {

  server.sendHeader(
    "Access-Control-Allow-Origin",
    "*"
  );

  server.sendHeader(
    "Access-Control-Allow-Methods",
    "GET, POST, OPTIONS"
  );

  server.sendHeader(
    "Access-Control-Allow-Headers",
    "Content-Type"
  );
}

// =====================================================
// FUNÇÃO PARA IDENTIFICAR A PESSOA
// =====================================================

String identificarPessoa(int id) {

  switch (id) {

    case 28:
      return "Sophia";

    case 27:
      return "Luís";

    case 3:
      return "Carlos";

    case 4:
      return "Ana";

    default:
      return "Desconhecido";
  }
}

// =====================================================
// ATUALIZAR DATA E HORA
// =====================================================

void atualizarDataHora() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {

    Serial.println("Erro ao obter data/hora");

    return;
  }

  char data[11];
  char hora[9];

  strftime(data, sizeof(data), "%d/%m/%Y", &timeinfo);
  strftime(hora, sizeof(hora), "%H:%M:%S", &timeinfo);

  dataAtual = String(data);
  horaAtual = String(hora);
}

// =====================================================
// GERAR JSON
// =====================================================

String gerarJSON() {

  atualizarDataHora();

  JsonDocument doc;

  doc["id"] = idPessoa;
  doc["nome"] = nomePessoa;
  doc["data"] = dataAtual;
  doc["hora"] = horaAtual;

  String json;

  serializeJson(doc, json);

  return json;
}

// =====================================================
// ROTA HTTP /DADOS
// =====================================================

void enviarDados() {

  String json = gerarJSON();

  // Adiciona os cabeçalhos CORS
  adicionarCORS();

  server.send(
    200,
    "application/json",
    json
  );
}

// =====================================================
// CONFIGURAÇÃO
// =====================================================

void setup() {

  Serial.begin(115200);

  // ---------------------------------------------------
  // SERIAL ENTRE ESP32 E ARDUINO
  // ---------------------------------------------------
  // RX2 do ESP32 = GPIO16
  // TX2 do ESP32 = GPIO17
  //
  // GPIO16 (RX2) recebe do TX2 do Arduino
  // GPIO17 (TX2) envia para o RX2 do Arduino
  // ---------------------------------------------------

  Serial2.begin(
    9600,
    SERIAL_8N1,
    16,   // RX2
    17    // TX2
  );

  // ---------------------------------------------------
  // WIFI
  // ---------------------------------------------------

  Serial.println();
  Serial.println("Conectando ao Wi-Fi...");

  WiFi.begin(
    ssid,
    password
  );

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado!");

  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  // ---------------------------------------------------
  // NTP
  // ---------------------------------------------------

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    ntpServer
  );

  Serial.println("Sincronizando data/hora...");

  delay(2000);

  atualizarDataHora();

  Serial.print("Data: ");
  Serial.println(dataAtual);

  Serial.print("Hora: ");
  Serial.println(horaAtual);

  // ---------------------------------------------------
  // SERVIDOR HTTP
  // ---------------------------------------------------

  server.on(
    "/dados",
    enviarDados
  );

  // Responde a requisições OPTIONS do navegador
  server.on(
    "/dados",
    HTTP_OPTIONS,
    []() {
      adicionarCORS();
      server.send(204);
    }
  );

  server.begin();

  Serial.println("Servidor HTTP iniciado!");
}

// =====================================================
// RECEBER ID DO ARDUINO
// =====================================================

void receberArduino() {

  if (Serial2.available()) {

    String mensagem = Serial2.readStringUntil('\n');

    mensagem.trim();

    if (mensagem.length() > 0) {

      int id = mensagem.toInt();

      idPessoa = id;

      nomePessoa = identificarPessoa(id);

      Serial.println("-----------------------------");
      Serial.println("Dados recebidos do Arduino");

      Serial.print("ID: ");
      Serial.println(idPessoa);

      Serial.print("Nome: ");
      Serial.println(nomePessoa);

      atualizarDataHora();

      Serial.print("Data: ");
      Serial.println(dataAtual);

      Serial.print("Hora: ");
      Serial.println(horaAtual);

      Serial.println("-----------------------------");

      // Mostra o JSON
      Serial.println(gerarJSON());
    }
  }
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Verifica se recebeu algo do Arduino
  receberArduino();

  // Processa requisições HTTP
  server.handleClient();
}