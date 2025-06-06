#include <PS4Controller.h>

// Pines de control de motores
#define AIN1 18
#define AIN2 5
#define PWMA 15

#define BIN1 27
#define BIN2 14
#define PWMB 2

#define AIN11 32
#define AIN22 33
#define PWMA1 12  // Motor lanzador 3 (PWM)

#define BIN11 25
#define BIN22 26
#define PWMB1 13  // Motor lanzador 4 (PWM)

// Variables para el control del PS4
int lY, rY;
bool L1, R1, L2, R2;
const int threshold = 20;

void setup() {
  Serial.begin(115200);
  PS4.begin();
  Serial.println("Esperando conexión del control PS4...");

  // Pines de motores de movimiento
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);

  // Pines de motores lanzadores
  pinMode(AIN11, OUTPUT);
  pinMode(AIN22, OUTPUT);
  pinMode(PWMA1, OUTPUT);

  pinMode(BIN11, OUTPUT);
  pinMode(BIN22, OUTPUT);
  pinMode(PWMB1, OUTPUT);
}

void loop() {
  if (PS4.isConnected()) {
    lY = PS4.LStickY();
    rY = PS4.RStickY();
    L1 = PS4.L1();
    R1 = PS4.R1();
    L2 = PS4.L2();
    R2 = PS4.R2();

    // Control de motores de movimiento
    controlarMotor(AIN1, AIN2, PWMA, lY);
    controlarMotor(BIN1, BIN2, PWMB, rY);

    // Control de lanzadores
    controlarLanzador(AIN11, AIN22, PWMA1, L1, L2);  // Lanzador izquierdo
    controlarLanzador(BIN11, BIN22, PWMB1, R1, R2);  // Lanzador derecho
  }
}

void controlarMotor(int IN1, int IN2, int PWM, int velocidad) {
  if (abs(velocidad) > threshold) {
    if (velocidad > 0) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    } else {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
    }
    analogWrite(PWM, map(abs(velocidad), 0, 128, 0, 255));
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(PWM, 0);
  }
}

void controlarLanzador(int IN1, int IN2, int PWM, bool botonAdelante, bool botonAtras) {
  if (botonAdelante) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(PWM, 255);  // Giro hacia adelante
  } 
  else if (botonAtras) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(PWM, 255);  // Giro hacia atrás
  } 
  else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(PWM, 0);    // Motor apagado
  }
}
