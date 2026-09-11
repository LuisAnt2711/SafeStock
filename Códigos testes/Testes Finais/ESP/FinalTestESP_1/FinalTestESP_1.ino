#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <time.h>

// =====================================================
// CONFIGURAÇÃO DO WI-FI
// =====================================================

const char* ssid = "moto g(10)_9303";
const char* password = "2711Luis";

// =====================================================
// CONFIGURAÇÃO DE DATA E HORA
// =====================================================

const long GMT_OFFSET_SEC = -3 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;

const char* ntpServer = "pool.ntp.org";

// =====================================================
// SERVIDOR WEB
// =====================================================

WebServer server(80);

// =====================================================
// VARIÁVEIS
// =====================================================

// ID e nome do usuário
int idPessoa = -1;
String nomePessoa = "Desconhecido";

// ID do remédio
int idRemedio = -1;

// Quantidade do remédio
int quantidade = 0;

// Data e hora
String dataAtual = "";
String horaAtual = "";

// Indica se existe uma nova informação recebida
bool dadosNovos = false;

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

    case 27:
      return "Luis Antonio Barbosa Pereira";

    case 28:
      return "Sophia Silva Faria";

    case 11:
      return "Nicoly Rocha dos Anjos";

    case 9:
      return "Elisa Rennó Costa";

    case 4:
      return "Lara Cecilia Santos";

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

  strftime(
    data,
    sizeof(data),
    "%d/%m/%Y",
    &timeinfo
  );

  strftime(
    hora,
    sizeof(hora),
    "%H:%M:%S",
    &timeinfo
  );

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
  doc["idRemedio"] = idRemedio;
  doc["quantidade"] = quantidade;
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

  // Adiciona os cabeçalhos CORS
  adicionarCORS();

  // ---------------------------------------------------
  // NÃO EXISTEM DADOS NOVOS
  // ---------------------------------------------------

  if (!dadosNovos) {

    server.send(204);

    return;
  }

  // ---------------------------------------------------
  // EXISTEM DADOS NOVOS
  // ---------------------------------------------------

  String json = gerarJSON();

  // Depois de enviar, marca como já enviado
  dadosNovos = false;

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
  //
  // RX2 = GPIO16
  // TX2 = GPIO17
  //
  // ---------------------------------------------------

  Serial2.begin(
    9600,
    SERIAL_8N1,
    16,
    17
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
// RECEBER DADOS DO ARDUINO
// =====================================================

void receberArduino() {

  if (Serial2.available()) {

    String mensagem = Serial2.readStringUntil('\n');

    mensagem.trim();

    if (mensagem.length() > 0) {

      // ------------------------------------------------
      // CRIA DOCUMENTO JSON
      // ------------------------------------------------

      JsonDocument doc;

      DeserializationError erro =
        deserializeJson(doc, mensagem);

      // ------------------------------------------------
      // VERIFICA SE O JSON É VÁLIDO
      // ------------------------------------------------

      if (erro) {

        Serial.println("-----------------------------");
        Serial.println("Erro ao interpretar JSON!");
        Serial.print("Mensagem recebida: ");
        Serial.println(mensagem);
        Serial.println("-----------------------------");

        return;
      }

      // ------------------------------------------------
      // RECEBE OS DADOS
      // ------------------------------------------------

      idPessoa = doc["idUsuario"] | -1;

      idRemedio = doc["idRemedio"] | -1;

      quantidade = doc["quantidade"] | 0;

      // Identifica o usuário
      nomePessoa = identificarPessoa(idPessoa);

      // Existe uma nova informação
      dadosNovos = true;

      // ------------------------------------------------
      // MOSTRA OS DADOS NO SERIAL
      // ------------------------------------------------

      Serial.println();
      Serial.println("=============================");
      Serial.println("DADOS RECEBIDOS DO ARDUINO");
      Serial.println("=============================");

      Serial.print("ID usuario: ");
      Serial.println(idPessoa);

      Serial.print("Nome: ");
      Serial.println(nomePessoa);

      Serial.print("ID remedio: ");
      Serial.println(idRemedio);

      Serial.print("Quantidade: ");
      Serial.println(quantidade);

      atualizarDataHora();

      Serial.print("Data: ");
      Serial.println(dataAtual);

      Serial.print("Hora: ");
      Serial.println(horaAtual);

      Serial.println("-----------------------------");

      // Mostra o JSON final
      Serial.println("JSON PARA O DASHBOARD:");

      Serial.println(gerarJSON());

      Serial.println("=============================");
    }
  }
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Verifica se recebeu dados do Arduino
  receberArduino();

  // Processa requisições HTTP
  server.handleClient();
}