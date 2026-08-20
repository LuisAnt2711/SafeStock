/* Teste LCD - SafeStock Logo
*/
// INCLUSÃO DE BIBLIOTECAS
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// DEFINIÇÕES
#define endereco  0x27
#define colunas   20
#define linhas    4

// INSTANCIANDO OBJETOS
LiquidCrystal_I2C lcd(endereco, colunas, linhas);

void setup() {

  // INICIALIZAÇÃO
  lcd.init();
  lcd.backlight();
  lcd.clear(); 

  // LOGO
  lcd.setCursor(0,0); lcd.print("....................");
  lcd.setCursor(0,1); lcd.print(".....Equipe2401.....");
  lcd.setCursor(0,2); lcd.print(".....SAFE-STOCK.....");
  lcd.setCursor(0,3); lcd.print("....................");

  delay(3000);

}

void loop() {

  int x = 0;

  // COBRE TUDO POR ZERO
  while(x!=20){
    lcd.setCursor(x,0); lcd.print("0");
    lcd.setCursor(x,1); lcd.print("0");
    lcd.setCursor(x,2); lcd.print("0");
    lcd.setCursor(x,3); lcd.print("0");
    delay(100);         x += 1;
  }
  delay(0); x = 0;

  // INICIO DA REESCRITA
  while(x!=5){ 
    // FRAME 
    lcd.setCursor(x,0); lcd.print(".");
    lcd.setCursor(x,1); lcd.print(".");
    lcd.setCursor(x,2); lcd.print(".");
    lcd.setCursor(x,3); lcd.print(".");
    delay(100);         x += 1;
  }

  // REESCRITA
      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("E");
      lcd.setCursor(x,2); lcd.print("S");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("q");
      lcd.setCursor(x,2); lcd.print("A");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("u");
      lcd.setCursor(x,2); lcd.print("F");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("i");
      lcd.setCursor(x,2); lcd.print("E");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("p");
      lcd.setCursor(x,2); lcd.print("-");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("e");
      lcd.setCursor(x,2); lcd.print("S");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("2");
      lcd.setCursor(x,2); lcd.print("T");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("4");
      lcd.setCursor(x,2); lcd.print("O");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("0");
      lcd.setCursor(x,2); lcd.print("C");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

      lcd.setCursor(x,0); lcd.print(".");
      lcd.setCursor(x,1); lcd.print("1");
      lcd.setCursor(x,2); lcd.print("K");
      lcd.setCursor(x,3); lcd.print(".");
      delay(100);         x += 1;

  // FINAL DA REESCRITA
  while(x!=20){
    // FRAMES FINAIS 
    lcd.setCursor(x,0); lcd.print(".");
    lcd.setCursor(x,1); lcd.print(".");
    lcd.setCursor(x,2); lcd.print(".");
    lcd.setCursor(x,3); lcd.print(".");
    delay(100);         x += 1;
  }
  delay(500);

}
