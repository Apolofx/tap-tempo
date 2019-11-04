/*
                        +-\/-+
  Ardu 10-------->CS   1|    |8  Vcc
  Ardu 13-------->SCK  2|    |7  Pot3 
  Ardu 11---> SDI/SDO  3|    |6  Pot2 
                  GND  4|    |5  Pot1 
                        +----+
*/
byte address = 0x00;
int CS= 10;
int dataPin = 11;
int clockPin = 13;
int sndPin = 7;
int rtnPin = A0;
long int sndTime;
long int rtnTime;
long int val;
long int duracion;
double delay_vector[256];
boolean calibracion = true;

void setup(){
  Serial.begin(9600);
  pinMode(sndPin, OUTPUT);
  pinMode(rtnPin, INPUT);
  pinMode (CS, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Serial.print("Comenzando rutina de calibracion en: ");
  Serial.print("3..");
  delay(1000);
  Serial.print("2..");
  delay(1000);
  Serial.print("1..");
  delay(1000);
}

void loop() {
  while(calibracion){
    
    for (int i = 0; i <= 255; i++){
    
      digitalPotWrite(i);
      digitalWrite(sndPin, 1);                      //envio pulso (5v rising)
      sndTime = millis();                           //Almacenamos el momento de disparo en sndTime
      delay(1);                                     //Espero 1 ms para que se estabilice
      while((millis() - sndTime) < 40){}            //Como el tiempo minimo de delay es de 50ms, me 
                                                    //aseguro de no hacer nada durante 40 ms aprox
      while(analogRead(rtnPin) == 0){}              //Se queda en este bucle hasta que la lectura da positivo
                                                    //IMPORTANTE: Pulldown Res entre rtnPin y GND
      while(1){                                     //creamos un nuevo bucle para leer la entrada analogica
        val = analogRead(rtnPin);                   //ponemos un valor de umbral y si una lectura
        //Serial.println(val); //Debug
        if (val >= 80){                            //supera ese umbral, anotamos el tiempo en que sucedio en la variable                                            
              /*Serial.print("el valor sensado es de ");   //rtnTime.
              float volts = val * (5.00 / 1023.00); 
              Serial.print(volts);
              Serial.println(" mV");*/
              rtnTime = millis(); 
              duracion = rtnTime - sndTime;             //Aca tenemos el valor de duracion que va a ir en la tabla de conversion
              delay_vector[i] = duracion;                                          //de tiempos--->digipotStep
              Serial.print("Paso Nº: ");
              Serial.println(i);
              Serial.print("El retardo es de ");       
              Serial.print(duracion);
              Serial.println(" ms");
              Serial.println("");
              Serial.println("");
              delay(200);             
              break;
            }             
          }
          digitalWrite(sndPin, 0);
          delay(1);
        }
calibracion = false;

for (int j = 0; j <= 255; j++){
  Serial.print(delay_vector[j]);
  Serial.print(", ");
      }
   }   
}



int digitalPotWrite(int value)
{
digitalWrite(CS, LOW);
shiftOut(dataPin, clockPin, MSBFIRST, 0b00010001);
shiftOut(dataPin, clockPin, MSBFIRST, value);
digitalWrite(CS, HIGH);
}
