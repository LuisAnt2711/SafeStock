#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

// =====================================================
// CONFIGURAÇÃO DAS PESSOAS
// =====================================================

const int idSophia = 28;
const String nameSophia = "Sophia Silva Faria";

const int idLuis = 27;
const String nameLuis = "Luis Antonio Barbosa Pereira";

// =====================================================
// CONFIGURAÇÃO DOS LEDs
// =====================================================

const int ledVerde = 3;
const int ledVermelho = 2;

// =====================================================
// BOTÃO DE ENCERRAR ACESSO
// =====================================================

const int BOTAO_ENCERRAR = 7;

// =====================================================
// SENSOR BIOMÉTRICO
// =====================================================

// Serial3 do Mega:
// RX3 = pino 15
// TX3 = pino 14

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);

// =====================================================
// LCD 20x4 I2C
// =====================================================

LiquidCrystal_I2C lcd(0x27, 20, 4);

// =====================================================
// TECLADO 4x3
// =====================================================

const byte LINHAS = 4;
const byte COLUNAS = 3;

char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte pinosLinhas[LINHAS] = {46, 50, 52, 42};
byte pinosColunas[COLUNAS] = {48, 40, 44};

Keypad teclado = Keypad(
  makeKeymap(teclas),
  pinosLinhas,
  pinosColunas,
  LINHAS,
  COLUNAS
);

// =====================================================
// ESTADOS DO SISTEMA
// =====================================================

enum Estado {
  AGUARDANDO_DIGITAL,
  DIGITANDO_REMEDIO,
  DIGITANDO_QUANTIDADE
};

Estado estadoAtual = AGUARDANDO_DIGITAL;

// =====================================================
// VARIÁVEIS DO ACESSO
// =====================================================

int idUsuario = -1;
int idRemedio = -1;
int quantidade = 0;

String valorDigitado = "";

// =====================================================
// SETUP
// =====================================================

void setup() {

  // ---------------------------------------------------
  // MONITOR SERIAL
  // ---------------------------------------------------

  Serial.begin(9600);

  while (!Serial);

  delay(100);

  // ---------------------------------------------------
  // SERIAL ENTRE MEGA E ESP32
  // ---------------------------------------------------

  // Serial2:
  // RX2 = pino 17
  // TX2 = pino 16

  Serial2.begin(9600);

  // ---------------------------------------------------
  // LEDs
  // ---------------------------------------------------

  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  digitalWrite(ledVerde, LOW);
  digitalWrite(ledVermelho, LOW);

  // ---------------------------------------------------
  // BOTÃO
  // ---------------------------------------------------

  pinMode(BOTAO_ENCERRAR, INPUT_PULLUP);

  // ---------------------------------------------------
  // LCD
  // ---------------------------------------------------

  lcd.init();
  lcd.backlight();

  // ---------------------------------------------------
  // SENSOR
  // ---------------------------------------------------

  Serial.println("\n\nTeste de Leitor Biometrico (AS608)");

  finger.begin(57600);

  if (finger.verifyPassword()) {

    Serial.println("Sensor encontrado com sucesso!");

  } else {

    Serial.println("Sensor nao encontrado :(");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erro no sensor!");

    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();

  Serial.print("O sensor contem ");
  Serial.print(finger.templateCount);
  Serial.println(" templates cadastrados.");

  mostrarAguardandoDigital();
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================

void loop() {

  // ===================================================
  // ESTADO 1 - AGUARDANDO DIGITAL
  // ===================================================

  if (estadoAtual == AGUARDANDO_DIGITAL) {

    int resultado = getFingerprintIDez();

    // -------------------------------------------------
    // DIGITAL RECONHECIDA
    // -------------------------------------------------

    if (resultado >= 0) {

      idUsuario = resultado;

      Serial.print("Digital encontrada! ID #");
      Serial.println(idUsuario);

      switch (idUsuario) {

        case 28:
          Serial.println(nameSophia);
          break;

        case 27:
          Serial.println(nameLuis);
          break;

        default:
          Serial.println("???");
          break;
      }

      // LED VERDE
      digitalWrite(ledVerde, HIGH);
      digitalWrite(ledVermelho, LOW);

      // Entra no estado de acesso
      estadoAtual = DIGITANDO_REMEDIO;

      valorDigitado = "";

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACESSO LIBERADO");

      lcd.setCursor(0, 1);
      lcd.print("Usuario ID: ");
      lcd.print(idUsuario);

      delay(1500);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ID do remedio:");

      lcd.setCursor(0, 1);
      lcd.print("> ");

      Serial.println("Acesso liberado.");
      Serial.println("Digite o ID do remedio.");

    }

    // -------------------------------------------------
    // DIGITAL NÃO CADASTRADA
    // -------------------------------------------------

    else if (resultado == -2) {

      Serial.println("Digital nao reconhecida!");

      digitalWrite(ledVermelho, HIGH);
      digitalWrite(ledVerde, LOW);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ACESSO NEGADO");

      lcd.setCursor(0, 1);
      lcd.print("Digital invalida");

      delay(2000);

      digitalWrite(ledVermelho, LOW);

      mostrarAguardandoDigital();
    }
  }

  // ===================================================
  // ESTADO 2 - DIGITANDO ID DO REMÉDIO
  // ===================================================

  else if (estadoAtual == DIGITANDO_REMEDIO) {

    char tecla = teclado.getKey();

    if (tecla) {

      // ------------------------------------------------
      // NÚMERO
      // ------------------------------------------------

      if (tecla >= '0' && tecla <= '9') {

        valorDigitado += tecla;

        lcd.setCursor(2, 1);
        lcd.print("                    ");

        lcd.setCursor(2, 1);
        lcd.print(valorDigitado);

        Serial.print(tecla);
      }

      // ------------------------------------------------
      // * = LIMPAR
      // ------------------------------------------------

      else if (tecla == '*') {

        valorDigitado = "";

        lcd.setCursor(2, 1);
        lcd.print("                  ");

        Serial.println();
        Serial.println("ID do remedio apagado.");
      }

      // ------------------------------------------------
      // # = CONFIRMAR
      // ------------------------------------------------

      else if (tecla == '#') {

        if (valorDigitado.length() > 0) {

          idRemedio = valorDigitado.toInt();

          Serial.println();
          Serial.print("ID do remedio: ");
          Serial.println(idRemedio);

          // Vai para quantidade
          estadoAtual = DIGITANDO_QUANTIDADE;

          valorDigitado = "";

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print("ID remedio: ");
          lcd.print(idRemedio);

          lcd.setCursor(0, 1);
          lcd.print("Quantidade:");

          lcd.setCursor(0, 2);
          lcd.print("> ");

          Serial.println("Digite a quantidade.");
        }
      }
    }
  }

  // ===================================================
  // ESTADO 3 - DIGITANDO QUANTIDADE
  // ===================================================

  else if (estadoAtual == DIGITANDO_QUANTIDADE) {

    char tecla = teclado.getKey();

    if (tecla) {

      // ------------------------------------------------
      // NÚMERO
      // ------------------------------------------------

      if (tecla >= '0' && tecla <= '9') {

        valorDigitado += tecla;

        lcd.setCursor(2, 2);
        lcd.print("                    ");

        lcd.setCursor(2, 2);
        lcd.print(valorDigitado);

        Serial.print(tecla);
      }

      // ------------------------------------------------
      // * = LIMPAR
      // ------------------------------------------------

      else if (tecla == '*') {

        valorDigitado = "";

        lcd.setCursor(2, 2);
        lcd.print("                  ");

        Serial.println();
        Serial.println("Quantidade apagada.");
      }

      // ------------------------------------------------
      // # = CONFIRMAR
      // ------------------------------------------------

      else if (tecla == '#') {

        if (valorDigitado.length() > 0) {

          quantidade = valorDigitado.toInt();

          Serial.println();
          Serial.print("Quantidade: ");
          Serial.println(quantidade);

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print("Dados registrados");

          lcd.setCursor(0, 1);
          lcd.print("Remedio: ");
          lcd.print(idRemedio);

          lcd.setCursor(0, 2);
          lcd.print("Qtd: ");
          lcd.print(quantidade);

          lcd.setCursor(0, 3);
          lcd.print("Pressione o botao");

          Serial.println("--------------------------------");
          Serial.println("Acesso em andamento.");
          Serial.println("Pressione o botao para encerrar.");
        }
      }
    }

    // -------------------------------------------------
    // BOTÃO ENCERRA O ACESSO
    // -------------------------------------------------

    if (digitalRead(BOTAO_ENCERRAR) == LOW) {

      delay(50); // debounce simples

      if (digitalRead(BOTAO_ENCERRAR) == LOW) {

        enviarDadosESP32();

        // Aguarda soltar o botão
        while (digitalRead(BOTAO_ENCERRAR) == LOW) {
          delay(10);
        }

        // Limpa os dados do acesso
        idUsuario = -1;
        idRemedio = -1;
        quantidade = 0;
        valorDigitado = "";

        // Desliga LED verde
        digitalWrite(ledVerde, LOW);

        // Volta para o início
        estadoAtual = AGUARDANDO_DIGITAL;

        mostrarAguardandoDigital();
      }
    }
  }

  delay(50);
}

// =====================================================
// MOSTRAR TELA INICIAL
// =====================================================

void mostrarAguardandoDigital() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Sistema de Acesso");

  lcd.setCursor(0, 2);
  lcd.print("Coloque a digital");

  Serial.println();
  Serial.println("Aguardando digital...");
}

// =====================================================
// ENVIAR DADOS PARA O ESP32
// =====================================================

void enviarDadosESP32() {

  // ---------------------------------------------------
  // JSON enviado pela Serial2
  // ---------------------------------------------------

  String json = "{";
  json += "\"idUsuario\":";
  json += idUsuario;
  json += ",";
  json += "\"idRemedio\":";
  json += idRemedio;
  json += ",";
  json += "\"quantidade\":";
  json += quantidade;
  json += "}";

  Serial2.println(json);

  Serial.println();
  Serial.println("================================");
  Serial.println("DADOS ENVIADOS AO ESP32:");
  Serial.println(json);
  Serial.println("================================");

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ACESSO ENCERRADO");

  lcd.setCursor(0, 1);
  lcd.print("Dados enviados!");

  delay(1500);
}

// =====================================================
// LEITURA DA DIGITAL
// =====================================================

int getFingerprintIDez() {

  uint8_t p = finger.getImage();

  if (p != FINGERPRINT_OK) {

    // Não há dedo no sensor
    return -1;
  }

  p = finger.image2Tz();

  if (p != FINGERPRINT_OK) {

    // Erro ao converter a imagem
    return -1;
  }

  p = finger.fingerSearch();

  if (p == FINGERPRINT_NOTFOUND) {

    // Digital lida, mas não cadastrada
    return -2;

  } else if (p != FINGERPRINT_OK) {

    // Outro erro
    return -1;
  }

  // Digital encontrada
  return finger.fingerID;
}