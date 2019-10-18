Rutinas de calibracion y de operacion para controlar el tempo del chip
de delay PT2399 mediante potenciometro digital.

**Calibracion:**
Tanto los PT2399 como los MCP41100 traen una dispersion de fabrica. 
En especial los MCP41100 he medido discrepancias de hasta un 30% con respecto
al valor declarado por el fabricante.
Por lo tanto es indispensable hacer un emparejamiento entre PT2399 y MCP41100.
Para esto vamosa necesitar el siguiente Hardware:
- Arduino cargado con el firmware de calibracion
- MCP41100 conectado a Arduino segun datasheet. Revisar que la libreria SPI.h 
funcione con este potenciometro. Sino, usar software SPI con ShiftOut, al igual
que como lo hicimos en la rutina de operacion en el Attiny
MOSI-DI SCL-CLK SS-CS. Los wipers del MCP van a ir conectados exactamente
igual a como se conectaria el potenciometro de carbon al PT2399.
- Arduino pin de señal conectado a Input del pedal.
 (Importante que en la calibracion, tanto el arduino como el pedal compartan la referencia 
a GND.  
- La salida del pedal se va a conectar al puerto ADC del arduino, estando este
conectado mediante una Pulldown de 1-10k a GND, a la espera de la señal 
retrasada.
 - Una vez que esta todo conectado, deberiamos alimentar el pedal a su alimentacion
de 9V.
- Abrir el Serial Monitor en el IDE, y reinciar arduino para que comience la rutina. 
- Finalizada la rutina, copiamos el array a excel. Como variable independiente
o eje x ponemos todos los tiempos sensados por el firmware. Como variable dependiente 
o eje y, ponemos todos los indices del array. Es decir, de 0 a 255.
- Generamos un polinomio de ajuste de grado 2 y copiamos los coeficientes 
a las variables (a, b, c) del firmware de operacion. 
- Programamos el Attiny85 con dicho firmware.
- Marcamos los 3 chips para saber que van en el mismo pedal.

**Operacion**
La rutina de operacion o funciomamiento normal del controlador es muy sensilla.
En el start-up del chip, el tempo asignado es el que este configurado en ese 
momento en el potenciometro del ADC.
Tenemos el tap-tempo sensado  mediante el switch, que esta atado a una interrupcion
externa (INT0) en el PB2 del micro. La interrupcion dispara (por flanco descendente) una ISR 
en donde solo cambiamos el valor la flag "botonPressed" a true. Esta sera redefinida 
a false cuando sea necesario dentro del codigo. 
En el main loop vamos a estar sensando todo el tiempo que el ADC del potenciometro
no se haya movido de su ultimo estado ("last_read" almacena el ultimo estado).
Si se movio, entonces traducimos la lectura ADC a un intervalo de tiempo, e ingresando
el tiempo determinado por el ADC al polinomio de ajuste, podemos transformar un valor
de tiempo en milisegundos a al byte que va a llevar el databyte en la comunicacion
con el potenciometro digital. 

*Aclaracion con respecto a la comunicacion con el MCP41100:
El Attiny no tiene harware SPI, usamos la USI mediante software SPI, utilizando 
la funcion ShiftOut, que se sincroniza con el CLK de ambos dispositivos para
enviar 2 bytes. El primer byte es el que se detalla en el datasheet del MCP41100
como el command byte. Este indica en que modo vamos a estar usandolo, y la direccion
de memoria que tiene asignado el potenciometro. En nuestro caso, el command byte
para nuestra aplicacion corresponde a 0b00010001.
El databyte, como ya explicamos, es la transformacion del tiempo a al dominio
de los steps del pote digital. Es decir, de un float de 50.0 a 600.0 ms, a un int
de 0 a 255.*

Por ultimo tenemos la rutina de subDivisiones. Esta se activa cuando se detecto que
movimos el potenciometro del ADC, PERO ademas de eso se encontraba previamente 
levantada la firstPress flag. Es decir, que para ingresar a esta rutina, apretamos
una vez el switch, vemos que el LED deja de parpadear, movemos el potenciometro, y cuando
el led parpadea rapidamente 3 veces, ya ingresamos en el modo de seleccion de subdivisiones.
para salir de dicho modo, solo volvemos a apretar el switch, y vamos a ver reflejado en el
parpadeo del LED, el nuevo tempo asignado por subDiv. 
****************************************************************************************
FALTA DESCRIBIR EL COMPORTAMIENTO POR TIME OVERFLOW
****************************************************************************************
