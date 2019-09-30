/***********************************************
              CIRCUITO DE APLICACION
************************************************
vcc---//47k//-----+||(1uF)---gnd        esto nos da aprox 1Tau = 47 ms suficiente para debouncear 
                |
                |-----Switch---gnd
                |
               R1k
                |
                |
            INT0 (PIN2)
                        +-\/-+
RESET  Ain0 (D 5) PB5  1|    |8  Vcc
CLK1   Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1  SCK  / USCK / SCL
CLK0   Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1  MISO / DO
                  GND  4|    |5  PB0 (D 0) pwm0  MOSI / DI / SDA
                        +----+
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "math.h"
/**************************************
DECLARACION/INICIALIZACION DE VARIABLES
****************************************/
byte tempo_Pot = 5, CS = 1, dataPin = 3, clockPin = 0, ledPin = 4, botonPin = 2;
int ledState = LOW, tempo,last_read, potRes_int;
float potRes, tappedTime, subDiv = 1, a = 0.0029, b = 0.7342, c = -45.2154;
unsigned long startCount, finishCount, previousMillis = 0;
boolean firstPress = false, secondPress = false, boot = true;
volatile boolean botonPress = false;
/************************************
               SETUP
************************************/
void setup()
{
  MCUCR |= (1 << ISC01); //Int en flanco descendente
  GIMSK |= (1 << INT0); //int externa
  SREG |= (1 << 7); // activamos el global interrupt
  DDRB |= (1 << ledPin) | (1 << CS) | (1 << dataPin) | (1 << clockPin); //Seteamos los outputs
  DDRB &= ~(1 << botonPin); // Boton en input
  PORTB |= (1 << botonPin); // input pull-ups
  ADCSRA |= (1 << 7);
  last_read = analogRead(5);
  tappedTime = last_read;
}

void loop()
{ 
  if (abs(analogRead(0) - last_read) >= 4){ 
    potTime(0); 
  }
  subDivision();
  ledBlink(tappedTime * subDiv); 
  if (botonPress == true && firstPress == false){
    // Detecta si se presiono el boton por 1ra vez
    startCount = millis(); // empieza el contador
    firstPress = true; // Flag de 1er pulsacion
    botonPress = false; // bajamos la Flag de boton presionado
  }
  if (firstPress == true && botonPress == true){
    finishCount = millis();
    tappedTime = finishCount - startCount;
    subDiv = 1;
    potRes = (a * pow(tappedTime, 2.0) + b * (tappedTime) + c);
    potRes_int = round(potRes); 
    if (potRes_int > 127){  
      potRes_int = 127;
    }
    digiPotWrite(potRes_int);
    finishCount = 0;
    startCount = 0;
    firstPress = false;
    botonPress = false;
  }
}

/***********************
        FUNCIONES
************************/

ISR(INT0_vect){
  botonPress = true;
}

void ledBlink(float time){
  // rutina de blink e indicador de sensando tiempo
  if (firstPress == true){
    PORTB &= ~(1 << ledPin);       //como el led RGB es anodo comun, tengo que poner el pin en
                                  // current sink (nivel logico 0 o LOW) para que encienda
  }
  else{
    unsigned long currentMillis = millis(); // Rutina de parpadeo de LED
    if (currentMillis - previousMillis >= time){
      previousMillis = currentMillis;
      PORTB ^= (1 << ledPin);
    }
  }
}
//potTime() usa el ADC para leer el potenciometro y lo mapea en un rango de
// 50 a 600 milisegundos para guardar el valor en tappedTime y ser usado tanto
// para la rutina del Led como para la conversion del tiempo a pot_resist del DigiPot
void potTime(byte x){
  tempo = map(analogRead(x), 455, 1023, 50, 600);
  tappedTime = tempo;
  subDiv = 1;
  potRes = (a * pow(tappedTime, 2.0) + b * (tappedTime) + c);
  potRes_int = round(potRes); 
  if (potRes_int > 127){  
    potRes_int = 127;
  }
  digiPotWrite(potRes_int);
  last_read = analogRead(x);
}
void digiPotWrite(int value){
    digitalWrite(CS, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, highByte(value));
    shiftOut(dataPin, clockPin, MSBFIRST, lowByte(value));
    digitalWrite(CS, HIGH);
}
void subDivision(){
  if (firstPress && (abs(analogRead(0) - last_read) >= 4)){
    firstPress = false;
    botonPress = false;
    digitalWrite(ledPin, LOW);
    while(1){
      int divRead = analogRead(0); 
      if (400 < divRead < 568){
        subDiv = 1; //'negra'
      }
      else if (568 < divRead < 681){
        subDiv = 0.5; //'corchea';
      }
      else if (681 < divRead < 794){
        subDiv = 0.25; //'semicorchea';
      }
      else if (794 < divRead < 907){
        subDiv = 0.75; //'corchea con puntillo';
      }
      else if (907 < divRead < 1023)
        subDiv = 0.33; // 'tresillo de corchea'
      else {
        return 0;
      }
      if (digitalRead(botonPin) == LOW){
        break;
      }
    }  
    potRes = (a * pow((tappedTime * subDiv), 2.0) + b * (tappedTime * subDiv) + c);
    potRes_int = round(potRes); 
    if (potRes_int > 127){  
      potRes_int = 127;
    }
    digiPotWrite(potRes_int);
    digitalWrite(3, LOW);
    delay(40);
    digitalWrite(3, HIGH);
    firstPress = false;
    botonPress = false;
    last_read = analogRead(0);
  }
}  


/************************************************************************
              C O D I G O  E N  C O N S T R U C C I O N
*************************************************************************/


/*
switch (subDiv) {
    case 1:
      potRes = (a * pow(tappedTime, 2.0) + b * (tappedTime) + c);
      potRes_int = round(potRes); 
      if (potRes_int > 127){  
          potRes_int = 127;
      }
      digiPotWrite(potRes_int);
      break;
    case 2:
      potRes = (a * pow(tappedTime * 0.5, 2.0) + b * (tappedTime * 0.5) + c);
      potRes_int = round(potRes); 
      if (potRes_int > 127){  
          potRes_int = 127;
      }
      digiPotWrite(potRes_int);
      break;
    case 3:
      potRes = (a * pow(tappedTime * 0.25, 2.0) + b * (tappedTime * 0.25) + c);
      potRes_int = round(potRes); 
      if (potRes_int > 127){  
          potRes_int = 127;
      }
      digiPotWrite(potRes_int);
      break;
    case 4:
      potRes = (a * pow(tappedTime * 0.75, 2.0) + b * (tappedTime * 0.75) + c);
      potRes_int = round(potRes); 
      if (potRes_int > 127){  
          potRes_int = 127;
      }
      digiPotWrite(potRes_int);
      break;
    case 5:
      potRes = (a * pow(tappedTime * 0.333, 2.0) + b * (tappedTime * 0.333) + c);
      potRes_int = round(potRes); 
      if (potRes_int > 127){  
          potRes_int = 127;
      }
      digiPotWrite(potRes_int);
      break;        
    default:
      return 0;
      // do something
}

*/