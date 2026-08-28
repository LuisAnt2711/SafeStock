int led_r = 3;
int led_g = 2;

void setup() {

  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);

  int i;

  for(i=0;i>10;i++){
    digitalWrite(led_g,1);
    digitalWrite(led_r,1);
    delay(500);
    digitalWrite(led_g,0);
    digitalWrite(led_r,0);
    digitalWrite(4,1);
  }

}

void loop() {
  digitalWrite(led_g,1);
  digitalWrite(led_r,0);
  delay(1500);
  digitalWrite(led_g,0);
  digitalWrite(led_r,1);
  delay(1500);

}
