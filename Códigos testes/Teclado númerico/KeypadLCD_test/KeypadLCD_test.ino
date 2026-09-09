#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

// =====================================================
// LCD 20x4 I2C
// =====================================================

LiquidCrystal_I2C lcd(0x27, 20, 4);

// =====================================================
// TECLADO 4x3 - 7 PINOS
// =====================================================

const byte LINHAS = 4;
const byte COLUNAS = 3;

char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// Altere estes pinos conforme sua ligação
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
// VARIÁVEL PARA ARMAZENAR O VALOR DIGITADO
// =====================================================

String valorDigitado = "";

void setup() {

  Serial.begin(9600);

  // Inicializa LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Digite um valor:");

  lcd.setCursor(0, 1);
  lcd.print("> ");
}

void loop() {

  char tecla = teclado.getKey();

  if (tecla) {

    // =================================================
    // NÚMEROS
    // =================================================

    if (tecla >= '0' && tecla <= '9') {

      valorDigitado += tecla;

      lcd.setCursor(2, 1);
      lcd.print(valorDigitado);

      Serial.print(tecla);
    }

    // =================================================
    // # = ENVIAR
    // =================================================

    else if (tecla == '#') {

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Valor enviado:");

      lcd.setCursor(0, 1);
      lcd.print(valorDigitado);

      Serial.println();
      Serial.print("Valor enviado: ");
      Serial.println(valorDigitado);
    }

    // =================================================
    // * = LIMPAR
    // =================================================

    else if (tecla == '*') {

      valorDigitado = "";

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Digite um valor:");

      lcd.setCursor(0, 1);
      lcd.print("> ");

      Serial.println();
      Serial.println("Valor apagado.");
    }
  }
}