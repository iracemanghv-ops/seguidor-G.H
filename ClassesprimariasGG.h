#ifndef CLASSESPRIMARIASGG_H
#define CLASSESPRIMARIASGG_H

#include <Arduino.h>

class Motor {
private:
  int in1;
  int in2;
  int enable;

public:
  Motor(int _in1, int _in2, int _enable)
    : in1(_in1), in2(_in2), enable(_enable) {}

  void begin() {
    pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(enable, OUTPUT);
  }

  void frente(int velocidade) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enable, constrain(velocidade, 0, 255));
  }

  void tras(int velocidade) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enable, constrain(velocidade, 0, 255));
  }

  void parar() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(enable, 0);
  }
};

class Rodas_Carro {
private:
  Motor& esquerdo;
  Motor& direito;

public:
  Rodas_Carro(Motor& _esquerdo, Motor& _direito)
    : esquerdo(_esquerdo), direito(_direito) {}

  void begin() {
    esquerdo.begin();
    direito.begin();
  }

  void frente(int velocidade) {
    esquerdo.frente(velocidade);
    direito.frente(velocidade);
  }

  void tras(int velocidade) {
    esquerdo.tras(velocidade);
    direito.tras(velocidade);
  }

  void esquerda_pouco(int velocidadeEsquerda) {
    esquerdo.frente(velocidadeEsquerda);
    direito.frente(150);
  }

  void esquerda_muito() {
    esquerdo.tras(70);
    direito.frente(180);
  }

  void direita_pouco(int velocidadeDireita) {
    esquerdo.frente(150);
    direito.frente(velocidadeDireita);
  }

  void direita_muito() {
    esquerdo.frente(180);
    direito.tras(70);
  }

  void girar_esquerda() {
    esquerdo.tras(100);
    direito.frente(180);
  }

  void girar_direita() {
    esquerdo.frente(180);
    direito.tras(100);
  }

  void parar() {
    esquerdo.parar();
    direito.parar();
  }
};

class Sensor_Distancia {
private:
  int trig;
  int echo;

public:
  Sensor_Distancia(int _trig, int _echo)
    : trig(_trig), echo(_echo) {}

  void begin() {
    pinMode(trig, OUTPUT);
    pinMode(echo, INPUT);
    digitalWrite(trig, LOW);
  }

  int get_distancia() {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);

    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    long duracao = pulseIn(echo, HIGH, 8000);

    if (duracao == 0) {
      return 999;
    }

    return duracao * 0.034 / 2;
  }
};

class Seguidor_Linha_ir {
private:
  int _pin_e;
  int _pin_c;
  int _pin_d;

public:
  Seguidor_Linha_ir(int p_e, int p_c, int p_d)
    : _pin_e(p_e), _pin_c(p_c), _pin_d(p_d) {}

  void begin() {
    pinMode(_pin_e, INPUT);
    pinMode(_pin_c, INPUT);
    pinMode(_pin_d, INPUT);
  }

  int esquerda() {
    return digitalRead(_pin_e);
  }

  int centro() {
    return digitalRead(_pin_c);
  }

  int direita() {
    return digitalRead(_pin_d);
  }

  char retornar_direcao() {
    int E = digitalRead(_pin_e);
    int C = digitalRead(_pin_c);
    int D = digitalRead(_pin_d);

    if (E == 1 && C == 0 && D == 1) return 'F';
    if (E == 0 && C == 0 && D == 1) return 'E';
    if (E == 0 && C == 1 && D == 1) return 'e';
    if (E == 1 && C == 0 && D == 0) return 'D';
    if (E == 1 && C == 1 && D == 0) return 'd';
    if (E == 0 && C == 0 && D == 0) return 'T';
    if (E == 1 && C == 1 && D == 1) return 'P';

    return '0';
  }
};

class Controle_Robo {
private:
  Motor esquerdo;
  Motor direito;
  Rodas_Carro rodas;
  Seguidor_Linha_ir seguidor_linha;
  Sensor_Distancia ultrassonico;

  int velocidade;
  int nivelCurva;
  unsigned long ultimaProgressao;
  const unsigned long intervaloProgressao = 35;

  int enrolando;
  unsigned long inicioEnrolamento;
  const unsigned long tempoMaximoEnrolamento = 1500;

  int distanciaLimite;

  const int velocidadeDesvio = 140;
  const unsigned long tempoGiro = 450;
  const unsigned long tempoLateral = 700;
  const unsigned long tempoPassar = 900;

  bool bloqueioObstaculo;
  unsigned long inicioBloqueioObstaculo;
  const unsigned long tempoBloqueioObstaculo = 1500;

public:
  Controle_Robo(int _velocidade)
    : esquerdo(13, 12, 11),
      direito(10, 8, 9),
      rodas(esquerdo, direito),
      seguidor_linha(A2, A1, A0),
      ultrassonico(7, 6),
      velocidade(_velocidade),
      nivelCurva(0),
      ultimaProgressao(0),
      enrolando(0),
      inicioEnrolamento(0),
      distanciaLimite(15),
      bloqueioObstaculo(false),
      inicioBloqueioObstaculo(0) {}

  void begin() {
    rodas.begin();
    seguidor_linha.begin();
    ultrassonico.begin();
    rodas.parar();
  }

  int ler_distancia() {
    return ultrassonico.get_distancia();
  }

  void curva_progressiva_esquerda() {
    if (millis() - ultimaProgressao >= intervaloProgressao) {
      ultimaProgressao = millis();

      if (nivelCurva < 5) {
        nivelCurva++;
      }
    }

    int velocidadeEsquerda;

    switch (nivelCurva) {
      case 0:
        velocidadeEsquerda = 135;
        break;

      case 1:
        velocidadeEsquerda = 125;
        break;

      case 2:
        velocidadeEsquerda = 115;
        break;

      case 3:
        velocidadeEsquerda = 100;
        break;

      case 4:
        velocidadeEsquerda = 80;
        break;

      default:
        velocidadeEsquerda = 60;
        break;
    }

    esquerdo.frente(velocidadeEsquerda);
    direito.frente(velocidade);
  }

  void curva_progressiva_direita() {
    if (millis() - ultimaProgressao >= intervaloProgressao) {
      ultimaProgressao = millis();

      if (nivelCurva < 5) {
        nivelCurva++;
      }
    }

    int velocidadeDireita;

    switch (nivelCurva) {
      case 0:
        velocidadeDireita = 135;
        break;

      case 1:
        velocidadeDireita = 125;
        break;

      case 2:
        velocidadeDireita = 115;
        break;

      case 3:
        velocidadeDireita = 100;
        break;

      case 4:
        velocidadeDireita = 80;
        break;

      default:
        velocidadeDireita = 60;
        break;
    }

    esquerdo.frente(velocidade);
    direito.frente(velocidadeDireita);
  }

  void desviar_obstaculo() {
    Serial.println();
    Serial.println("==============================");
    Serial.println("   OBSTACULO DETECTADO");
    Serial.println("==============================");

    Serial.println("1 - PARANDO");
    rodas.parar();
    delay(300);

    Serial.println("2 - GIRANDO ESQUERDA");
    rodas.girar_esquerda();
    delay(tempoGiro);
    rodas.parar();
    delay(150);

    Serial.println("3 - INDO PARA FRENTE");
    rodas.frente(velocidadeDesvio);
    delay(tempoLateral);
    rodas.parar();
    delay(150);

    Serial.println("4 - GIRANDO DIREITA");
    rodas.girar_direita();
    delay(tempoGiro);
    rodas.parar();
    delay(150);

    Serial.println("5 - PASSANDO OBSTACULO");
    rodas.frente(velocidadeDesvio);
    delay(tempoPassar);
    rodas.parar();
    delay(150);

    Serial.println("6 - GIRANDO DIREITA");
    rodas.girar_direita();
    delay(tempoGiro);
    rodas.parar();
    delay(150);

    Serial.println("7 - VOLTANDO PARA LINHA");
    rodas.frente(velocidadeDesvio);
    delay(tempoLateral);
    rodas.parar();
    delay(150);

    Serial.println("8 - ALINHANDO COM LINHA");
    rodas.girar_esquerda();
    delay(tempoGiro);
    rodas.parar();
    delay(150);

    Serial.println("==============================");
    Serial.println("      DESVIO FINALIZADO");
    Serial.println("==============================");
    Serial.println();

    nivelCurva = 0;
    ultimaProgressao = millis();
    enrolando = 0;

    bloqueioObstaculo = true;
    inicioBloqueioObstaculo = millis();
  }

  void seguir_linha() {
    int E = seguidor_linha.esquerda();
    int C = seguidor_linha.centro();
    int D = seguidor_linha.direita();

    Serial.print("E=");
    Serial.print(E);
    Serial.print(" C=");
    Serial.print(C);
    Serial.print(" D=");
    Serial.println(D);

    if (enrolando == -1) {
      if (millis() - inicioEnrolamento >= tempoMaximoEnrolamento) {
        rodas.parar();
        enrolando = 0;
        Serial.println("LIMITE DE GIRO ESQUERDA");
        return;
      }

      if (E == 1 && C == 0 && D == 1) {
        enrolando = 0;
        nivelCurva = 0;
        ultimaProgressao = millis();
        rodas.frente(velocidade);
        return;
      }

      rodas.esquerda_muito();
      return;
    }

    if (enrolando == 1) {
      if (millis() - inicioEnrolamento >= tempoMaximoEnrolamento) {
        rodas.parar();
        enrolando = 0;
        Serial.println("LIMITE DE GIRO DIREITA");
        return;
      }

      if (E == 1 && C == 0 && D == 1) {
        enrolando = 0;
        nivelCurva = 0;
        ultimaProgressao = millis();
        rodas.frente(velocidade);
        return;
      }

      rodas.direita_muito();
      return;
    }

    if (E == 1 && C == 0 && D == 1) {
      nivelCurva = 0;
      ultimaProgressao = millis();
      rodas.frente(velocidade);
      return;
    }

    if (E == 0 && C == 0 && D == 1) {
      curva_progressiva_esquerda();
      return;
    }

    if (E == 0 && C == 1 && D == 1) {
      enrolando = -1;
      inicioEnrolamento = millis();
      nivelCurva = 0;
      rodas.esquerda_muito();
      return;
    }

    if (E == 1 && C == 0 && D == 0) {
      curva_progressiva_direita();
      return;
    }

    if (E == 1 && C == 1 && D == 0) {
      enrolando = 1;
      inicioEnrolamento = millis();
      nivelCurva = 0;
      rodas.direita_muito();
      return;
    }

    if (E == 0 && C == 0 && D == 0) {
      nivelCurva = 0;
      rodas.frente(110);
      return;
    }

    if (E == 1 && C == 1 && D == 1) {
      nivelCurva = 0;
      rodas.frente(100);
      return;
    }

    nivelCurva = 0;
    rodas.frente(90);
  }

  void atualizar() {
    int distancia = ler_distancia();

    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" cm");

    if (bloqueioObstaculo &&
        millis() - inicioBloqueioObstaculo >= tempoBloqueioObstaculo) {
      bloqueioObstaculo = false;
    }

    if (!bloqueioObstaculo &&
        distancia <= distanciaLimite) {
      desviar_obstaculo();
      return;
    }

    seguir_linha();
  }

  void parar() {
    rodas.parar();
  }
};

#endif
