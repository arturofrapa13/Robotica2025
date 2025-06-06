#include <PS4Controller.h>

// Pines de control de motores de ruedas
#define AIN1 27
#define AIN2 14
#define PWMA 2  // PWM motor 1 (rueda izquierda)

#define BIN1 27
#define BIN2 14
#define PWMB 2   // PWM motor 2 (rueda derecha)

// Pines de motores lanzadores (robofut)
#define AIN11 18
#define AIN22 5
#define PWMA1 15  // PWM motor 3 (lanzador izquierdo)zs

#define BIN11 25
#define BIN22 26
#define PWMB1 13  // PWM motor 4 (lanzador derecho)


const int threshold = 20;
bool lanzadoresActivos = true;

// Variables para detectar cambios y evitar mensajes repetidos
bool lastTriangle = false;
bool lastL1 = false;
bool lastR1 = false;
bool lastUp = false;
bool lastDown = false;
bool lastLeft = false;
bool lastRight = false;
int lastR2 = 0;
int lastL2 = 0;
int lastJoyX = 0;
int lastJoyY = 0;

void setup() {
  Serial.begin(115200);
  PS4.begin();
  Serial.println("Esperando conexión del control PS4...");

  // Pines de motores de movimiento
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);

  // Pines de lanzadoras
  pinMode(AIN11, OUTPUT); pinMode(AIN22, OUTPUT); pinMode(PWMA1, OUTPUT);
  pinMode(BIN11, OUTPUT); pinMode(BIN22, OUTPUT); pinMode(PWMB1, OUTPUT);
}

void loop() {
  if (PS4.isConnected()) {
    int velocidad = 0;
    int giro = 0;

    // --- Mensajes de botones y joystick ---
    // Triángulo detiene/activa lanzadoras (solo al presionar, no mantener)
    if (PS4.Triangle() && !lastTriangle) {
      lanzadoresActivos = !lanzadoresActivos;
      Serial.print("[TRIANGULO] Lanzadores ");
      Serial.println(lanzadoresActivos ? "ACTIVADOS" : "DESACTIVADOS");
      delay(300); // Evita rebotes
    }
    lastTriangle = PS4.Triangle();

    // L1 y R1 para lanzadores
    if (PS4.L1() && !lastL1) {
      Serial.println("[L1] Lanzador izquierdo ACTIVADO");
    } else if (!PS4.L1() && lastL1) {
      Serial.println("[L1] Lanzador izquierdo DESACTIVADO");
    }
    lastL1 = PS4.L1();

    if (PS4.R1() && !lastR1) {
      Serial.println("[R1] Lanzador derecho ACTIVADO");
    } else if (!PS4.R1() && lastR1) {
      Serial.println("[R1] Lanzador derecho DESACTIVADO");
    }
    lastR1 = PS4.R1();

    // Flechas de dirección
    if (PS4.Up() && !lastUp) Serial.println("[FLECHA ARRIBA] Movimiento adelante");
    if (!PS4.Up() && lastUp) Serial.println("[FLECHA ARRIBA] Liberada");
    lastUp = PS4.Up();

    if (PS4.Down() && !lastDown) Serial.println("[FLECHA ABAJO] Movimiento atrás");
    if (!PS4.Down() && lastDown) Serial.println("[FLECHA ABAJO] Liberada");
    lastDown = PS4.Down();

    if (PS4.Left() && !lastLeft) Serial.println("[FLECHA IZQUIERDA] Giro izquierda");
    if (!PS4.Left() && lastLeft) Serial.println("[FLECHA IZQUIERDA] Liberada");
    lastLeft = PS4.Left();

    if (PS4.Right() && !lastRight) Serial.println("[FLECHA DERECHA] Giro derecha");
    if (!PS4.Right() && lastRight) Serial.println("[FLECHA DERECHA] Liberada");
    lastRight = PS4.Right();

    // Gatillos analógicos R2/L2
    if (PS4.R2Value() != lastR2) {
      if (PS4.R2Value() > 0) {
        Serial.print("[R2] Avance: ");
        Serial.println(PS4.R2Value());
      }
      lastR2 = PS4.R2Value();
    }
    if (PS4.L2Value() != lastL2) {
      if (PS4.L2Value() > 0) {
        Serial.print("[L2] Retroceso: ");
        Serial.println(PS4.L2Value());
      }
      lastL2 = PS4.L2Value();
    }

    // Joystick izquierdo (solo si se mueve significativamente)
    int ejeX = PS4.LStickX();
    int ejeY = PS4.LStickY();
    if (abs(ejeX - lastJoyX) > 10 || abs(ejeY - lastJoyY) > 10) {
      if (abs(ejeX) > 10 || abs(ejeY) > 10) {
        Serial.print("[JOYSTICK] X: ");
        Serial.print(ejeX);
        Serial.print(" | Y: ");
        Serial.println(ejeY);
      }
      lastJoyX = ejeX;
      lastJoyY = ejeY;
    }

    // Control de movimiento con R2/L2 (adelante/atrás con stick izquierdo de dirección)
    if (PS4.R2()) velocidad = PS4.R2Value();       // Avanza
    else if (PS4.L2()) velocidad = -PS4.L2Value(); // Retrocede

    // Solo si hay velocidad (R2/L2 pulsados), el stick izquierdo controla el giro
    if (velocidad != 0) {
      giro = map(PS4.LStickX(), -128, 127, -100, 100);
      if (velocidad > 0) giro = -giro; // Invertimos sentido de giro si va hacia adelante
    }
    // Si no se usa R2/L2, se prioriza el control con flechas (movimiento tradicional)
    else {
      if (PS4.Up()) velocidad = 255;           // Adelante
      else if (PS4.Down()) velocidad = -255;   // Atrás

      if (PS4.Left()) giro = -100;             // Giro izquierda
      else if (PS4.Right()) giro = 100;        // Giro derecha
    }

    // Control proporcional de dirección
    int izquierda = velocidad - giro;
    int derecha = velocidad + giro;

    moverMotor(AIN1, AIN2, PWMA, izquierda);
    moverMotor(BIN1, BIN2, PWMB, derecha);

    // Lanzadores (solo si están activos)
    if (lanzadoresActivos) {
      if (PS4.L1()) {
        lanzar(AIN11, AIN22, PWMA1, true);  // Izquierdo adelante
      } else {
        apagarLanzador(AIN11, AIN22, PWMA1);
      }

      if (PS4.R1()) {
        lanzar(BIN11, BIN22, PWMB1, false); // Derecho atrás
      } else {
        apagarLanzador(BIN11, BIN22, PWMB1);
      }
    } else {
      // Apaga lanzadores si están desactivados
      apagarLanzador(AIN11, AIN22, PWMA1);
      apagarLanzador(BIN11, BIN22, PWMB1);
    }
  }
}

void moverMotor(int IN1, int IN2, int PWM, int velocidad) {
  velocidad = constrain(velocidad, -255, 255);
  if (abs(velocidad) > threshold) {
    digitalWrite(IN1, velocidad > 0);
    digitalWrite(IN2, velocidad < 0);
    analogWrite(PWM, map(abs(velocidad), 0, 255, 0, 255));
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(PWM, 0);
  }
}

void lanzar(int IN1, int IN2, int PWM, bool adelante) {
  digitalWrite(IN1, adelante);
  digitalWrite(IN2, !adelante);
  analogWrite(PWM, 255);
}

void apagarLanzador(int IN1, int IN2, int PWM) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(PWM, 0);
}
