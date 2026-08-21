/* Teste LCD - Spiral
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
  delay(2000);
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop() {

  int z;
  int x = 0;
  int y = 0;
  
  // zera
  while(x<20){
    lcd.setCursor(x,y); lcd.print(z); x+=1;
    delay(100);
  }x = 19; y = 0;
  while(y<4){
    lcd.setCursor(x,y); lcd.print(z); y+=1;
    delay(100);
  }x = 19; y = 3;
  while(x>-1){
    lcd.setCursor(x,y); lcd.print(z); x-=1;
    delay(100);
  }x = 0; y = 3;
  while(y>-1){
    lcd.setCursor(x,y); lcd.print(z); y-=1;
    delay(100);
  }x = 1; y = 1;
  while(x<19){
    lcd.setCursor(x,y); lcd.print(z); x+=1;
    delay(100);
  }x = 18; y = 2;
  while(x>0){
    lcd.setCursor(x,y); lcd.print(z); x-=1;
    delay(100);
  }x = 0; y = 3;
  if(z>9){z=0;}

}
