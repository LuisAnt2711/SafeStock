#include <Adafruit_Fingerprint.h>

// Definindo os pinos dos LEDs
const int ledVerde = 3;
const int ledVermelho = 2;

// Para Arduino Mega, usamos Serial1 (Pino 19 como RX e Pino 18 como TX)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);

void setup() {
  // Configura a velocidade do monitor serial do computador
  Serial.begin(9600);
  while (!Serial);  // Espera o Serial abrir (necessário para alguns Arduinos)
  delay(100);

  // Configura os pinos dos LEDs como saída
  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);

  // Garante que começam desligados
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledVermelho, LOW);

  Serial.println("\n\nTeste de Leitor Biometrico (AS608)");

  // Configura a taxa de transmissão do sensor (geralmente 57600)
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
  // Tenta ler a digital. Se retornar FINGERPRINT_OK, a digital foi encontrada.
  int resultado = getFingerprintIDez();
  
  if (resultado >= 0) {
    // DIGITAL RECONHECIDA
    Serial.print("Digital encontrada! ID #"); Serial.println(resultado);
    
    // Acende o LED verde e apaga o vermelho por 2 segundos
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledVermelho, LOW);
    delay(2000);
    
    // Desliga o LED verde após o tempo
    digitalWrite(ledVerde, LOW);
  } else {
    // DIGITAL NÃO RECONHECIDA (ou nenhuma digital colocada ainda)
    // Nota: A função abaixo roda continuamente, então colocamos o alerta visual
    // apenas quando houver tentativa falha ou podemos deixar o vermelho piscar/acender se houver erro de leitura.
  }
  
  delay(50); // Pequena pausa para estabilidade
}

// Função que verifica a digital e retorna o ID ou -1 se falhar
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    // Se não há dedo no sensor, não faz nada
    return -1;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    // Erro ao converter a imagem
    piscarVermelho();
    return -1;
  }

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) {
    // Digital não encontrada na base de dados (Não reconhecida)
    piscarVermelho();
    return -1;
  }

  // Se chegou aqui, encontrou! Retorna o ID da digital
  return finger.fingerID;
}

// Função auxiliar para piscar o LED vermelho em caso de erro/digital não reconhecida
void piscarVermelho() {
  digitalWrite(ledVermelho, HIGH);
  digitalWrite(ledVerde, LOW);
  delay(1000);
  digitalWrite(ledVermelho, LOW);
}