#include "ClassesprimariasGG.h"

Controle_Robo robo(122);

void setup() {
  Serial.begin(115200);
  robo.begin();
  delay(1000);
}

void loop() {
  robo.atualizar();
  delay(5);
}