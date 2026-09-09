#include <Adafruit_Fingerprint.h>

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
// SENSOR BIOMÉTRICO
// =====================================================

// Serial3 do Mega:
// RX3 = pino 15
// TX3 = pino 14

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);

// =====================================================
// CONFIGURAÇÃO
// =====================================================

void setup() {

  // ---------------------------------------------------
  // MONITOR SERIAL DO COMPUTADOR
  // ---------------------------------------------------

  Serial.begin(9600);

  while (!Serial);

  delay(100);

  // ---------------------------------------------------
  // SERIAL ENTRE MEGA E ESP32
  // ---------------------------------------------------
  //
  // Serial2:
  // RX2 = pino 17
  // TX2 = pino 16
  //
  // TX2 do Mega -> RX2 do ESP32
  // RX2 do Mega <- TX2 do ESP32
  //
  // ---------------------------------------------------

  Serial2.begin(9600);

  // ---------------------------------------------------
  // LEDs
  // ---------------------------------------------------

  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  digitalWrite(ledVerde, LOW);
  digitalWrite(ledVermelho, LOW);

  // ---------------------------------------------------
  // SENSOR
  // ---------------------------------------------------

  Serial.println("\n\nTeste de Leitor Biometrico (AS608)");

  finger.begin(57600);

  if (finger.verifyPassword()) {

    Serial.println("Sensor encontrado com sucesso!");

  } else {

    Serial.println("Sensor nao encontrado :(");

    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();

  Serial.print("O sensor contem ");
  Serial.print(finger.templateCount);
  Serial.println(" templates cadastrados.");

  Serial.println("Aguardando digital...");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  int resultado = getFingerprintIDez();

  // ===================================================
  // DIGITAL RECONHECIDA
  // ===================================================

  if (resultado >= 0) {

    Serial.print("Digital encontrada! ID #");
    Serial.println(resultado);

    switch (resultado) {

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

    // -------------------------------------------------
    // ENVIA O ID PARA O ESP32
    // -------------------------------------------------

    Serial2.println(resultado);

    Serial.print("ID enviado ao ESP32: ");
    Serial.println(resultado);

    // -------------------------------------------------
    // LED VERDE
    // -------------------------------------------------

    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledVermelho, LOW);

    delay(5000);

    digitalWrite(ledVerde, LOW);
  }

  // ===================================================
  // DIGITAL NÃO CADASTRADA
  // ===================================================

  else if (resultado == -2) {

    Serial.println("Digital nao reconhecida!");

    // -------------------------------------------------
    // LED VERMELHO
    // -------------------------------------------------

    digitalWrite(ledVermelho, HIGH);
    digitalWrite(ledVerde, LOW);

    delay(5000);

    digitalWrite(ledVermelho, LOW);
  }

  delay(50);
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