#include <Adafruit_Fingerprint.h>

// Definindo os pinos dos LEDs
const int idSophia = 28;
const String nameSophia = "Sophia Silva Faria";
const int idLuis = 27;
const String nameLuis = "Luis Antonio Barbosa Pereira";
const int ledVerde = 3;
const int ledVermelho = 2;

// Para Arduino Mega, usamos Serial1 (Pino 19 como RX e Pino 18 como TX)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);

void setup() {
  // Configura a velocidade do monitor serial do computador
  Serial.begin(9600);
  while (!Serial);  // Espera o Serial abrir
  delay(100);

  // Configura os pinos dos LEDs como saída
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  // Garante que começam desligados
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledVermelho, LOW);

  Serial.println("\n\nTeste de Leitor Biometrico (AS608)");

  // Configura a taxa de transmissão do sensor
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Sensor encontrado com sucesso!");
  } else {
    Serial.println("Sensor nao encontrado :(");
    while (1) { delay(1); }
  }

  finger.getTemplateCount();
  Serial.print("O sensor contem "); Serial.print(finger.templateCount); Serial.println(" templates cadastrados.");
  Serial.println("Aguardando digital...");
}

void loop() {
  int resultado = getFingerprintIDez();
  
  if (resultado >= 0) {
    // DIGITAL RECONHECIDA
    Serial.print("Digital encontrada! ID #"); Serial.println(resultado);
    switch(resultado){
      case 28:
      Serial.println(nameSophia); break;
      case 27:
      Serial.println(nameLuis); break;
      default:
      Serial.println("???"); break;
    }
    
    // Acende o LED verde por 2 segundos e garante o vermelho apagado
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledVermelho, LOW);
    delay(5000);
    digitalWrite(ledVerde, LOW);
  } 
  else if (resultado == -2) {
    // DIGITAL NEGADA (Não cadastrada)
    Serial.println("Digital nao reconhecida!");
    
    // Acende o LED vermelho por 1 segundo
    digitalWrite(ledVermelho, HIGH);
    digitalWrite(ledVerde, LOW);
    delay(5000);
    digitalWrite(ledVermelho, LOW);
  }
  
  delay(50); // Pequena pausa para estabilidade
}

// Função modificada para diferenciar erro de leitura de digital negada
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    // Se não há dedo no sensor, retorna -1 (silencioso)
    return -1;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    // Erro ao converter a imagem, retorna -1 (silencioso)
    return -1;
  }

  // Usando o nome correto da função corrigido anteriormente
  p = finger.fingerSearch();
  if (p == FINGERPRINT_NOTFOUND) {
    // Digital escaneada, mas NÃO encontrada no banco de dados
    return -2; 
  } else if (p != FINGERPRINT_OK) {
    // Outro tipo de erro genérico de busca, retorna -1 (silencioso)
    return -1;
  }

  // Se chegou aqui, encontrou a digital com sucesso! Retorna o ID
  return finger.fingerID;
}