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
int ledState = LOW, tempo, last_read, potRes_int;
float potRes, tappedTime, subDiv = 1.0, a = 1.2813108742542093e-05, b = 0.22416, c = -14.12274;
unsigned long startCount, finishCount, previousMillis = 0;
boolean firstPress = false, secondPress = false, boot = true;
volatile boolean botonPress = false;
/************************************
                SETUP
************************************/
void setup()
{
  MCUCR |= (1 << ISC01); // Int en flanco descendente
  GIMSK |= (1 << INT0); // int externa
  SREG |= (1 << 7); // activamos el global interrupt
  DDRB |= (1 << ledPin) | (1 << CS) | (1 << dataPin) | (1 << clockPin); // Seteamos los outputs
  DDRB &= ~(1 << botonPin); // Boton en input
  // PORTB |= (1 << botonPin); // input pull-ups desactivadas mientras tengasmos la external pullup
  ADCSRA |= (1 << 7);
  last_read = analogRead(A0);
  tappedTime = map(last_read, 450, 1023, 57, 600);
  digiPotWrite(tappedByte());
}
/************************************
            MAIN LOOP
************************************/
void loop()
{
  if (abs(analogRead(A0) - last_read) >= 4)
  {
    if (firstPress)
    {
      ledflash(3);
      subDiv = subDivision();
      firstPress = false;
      botonPress = false;
    }
    else
    {
      potTime(A0);
    }
  }
  ledBlink(tappedTime, subDiv);
  if (botonPress && !firstPress)
  {
    // Detecta si se presiono el boton por 1ra vez
    startCount = millis(); // empieza el contador
    firstPress = true; // Flag de 1er pulsacion
    botonPress = false; // bajamos la Flag de boton presionado
  }
  if (firstPress && botonPress)
  {
    finishCount = millis();
    firstPress = false;
    botonPress = false;
    if ((finishCount - startCount) < 1000)
    {
      if ((finishCount - startCount) <= 655)
      {
        tappedTime = finishCount - startCount;
      }
      else
      {
        tappedTime = 655;
      }
      subDiv = 1.0;
      potRes = (a * pow(tappedTime, 2.0) + b * (tappedTime) + c);
      potRes_int = round(potRes);
      digiPotWrite(potRes_int);
    }
      //MODO ROBOT
    else
    { 
      ledflash(3);
      int speed;
      MCUCR &= ~(1 << ISC01); //Desactivamos interrupciones
      GIMSK &= ~(1 << INT0); 
      delay(50);
      while (digitalRead(botonPin) == HIGH)
      { 
        speed = map(analogRead(A0), 450, 1023, 1, 1000);
        digiPotWrite(int(random(0, 130)));
        delay(speed);
      }
      digiPotWrite(potRes_int);
      delay(100);
      MCUCR |= (1 << ISC01); // activamos interrupciones
      GIMSK |= (1 << INT0); 
    }
  }
}


/***********************
        FUNCIONES
************************/
ISR(INT0_vect)
{
  botonPress = true;
}

void ledBlink(float time, float div)
{
  // rutina de blink e indicador de sensando tiempo
  if (firstPress == true)
  {
    PORTB |= (1 << ledPin); // como el led RGB es anodo comun, tengo que poner el pin en
    // current sink (nivel logico 0 o LOW) para que encienda
  }
  else
  { 
    unsigned long currentMillis = millis(); // Rutina de parpadeo de LED
    if ((currentMillis - previousMillis) >= round(time*div))
    {
      previousMillis = currentMillis;
      PORTB ^= (1 << ledPin);
    }

  }
}

void potTime(byte x)
/*potTime() usa el ADC para leer el potenciometro y lo mapea en un rango de
  50 a 600 milisegundos para guardar el valor en tappedTime y ser usado tanto
  para la rutina del Led como para la conversion del tiempo a pot_resist del DigiPot*/
{
  tempo = map(analogRead(x), 450, 1023, 57, 600);
  tappedTime = tempo;
  subDiv = 1;
  potRes = (a * pow(tappedTime, 2.0) + b * (tappedTime) + c);
  potRes_int = round(potRes);
  //Overflow:
  if (potRes_int > 255)
  {
    potRes_int = 128;
  }
  else if (potRes_int < 0)
  {
    potRes_int = 0;
  }

  digiPotWrite(potRes_int);
  last_read = analogRead(x);
}

void digiPotWrite(int value)
{
  digitalWrite(CS, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, 0b00010001);
  shiftOut(dataPin, clockPin, MSBFIRST, lowByte(value));
  digitalWrite(CS, HIGH);
}

float subDivision()
{
  MCUCR &= ~(1 << ISC01); // Int en flanco descendente
  GIMSK &= ~(1 << INT0); // int externa
  while (digitalRead(botonPin) == HIGH)
  {
    int read = analogRead(A0);
    if (read <= 597)
    {
      subDiv = 1.0; // 'negra'
    }
    else
      if (597 < read && read <= 703)
      {
        subDiv = 0.75; // 'corchea con puntillo';
      }
    else
      if (703 < read && read <= 809)
      {
        subDiv = 0.5; // 'corchea';
      }
    else
      if (809 < read && read <= 916)
      {
        subDiv = 0.33; // 'tresillo de corchea';
      }
    else
      if (916 < read && read <= 1023)
      {
        subDiv = 0.25; // 'semicorchea'
      }
  }
  delay(50);
  potRes = (a * pow((tappedTime * subDiv), 2.0) + b * (tappedTime * subDiv) + c);
  potRes_int = round(potRes);
  if (potRes_int > 255)
  {
    potRes_int = 255;
  }
  digiPotWrite(potRes_int);
  last_read = analogRead(A0);
  MCUCR |= (1 << ISC01); // Int en flanco descendente
  GIMSK |= (1 << INT0); // int externa
  return subDiv;
}

void ledflash(int n)
{
  for (int i = 0; i < n; i++)
  {
    PORTB^= (1 << ledPin);
    delay(50);
    PORTB^= (1 << ledPin);
    delay(30);
  }
}

byte tappedByte(){
  potRes = (a * pow(tappedTime, 2.0) + b * (tappedTime) + c);
  potRes_int = round(potRes);
  if (potRes_int > 255)
  {
    potRes_int = 255;
  }
  return potRes_int;
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