1. Control por tap	OK (habria que ver si se cambiar de 2 taps a 3, esto involucraria el agregado de un contador, y un promedio)
2. Control por pot	OK
3. Comunicacion SPI con Digipot OK:
	*Aca teniamos mal el command byte. En los MCP41xxx, para modo Write y elecicon de Pote nro 1
	se necesita mandar el command byte 0b00010001 primero y despues el data Byte*
4. Control por subdivision OK
5. Overflow: OK
	**Problema/Hipotesis:**
		*El over flow esta controlado solo para cuando se introdujo mediante tap un tempo superior al permitido por el chip. 
		faltaria agregar una rutina que chequea que ya se realizo la primer pisada correspondiente a un tap-tempo, pero si despues de x cantidad de segundos no se hace nada, dejar de esperar un segundo tap. Esto implicaria lo siguiente.
			main loop: 
				void second_tap_OverFlow():
					if firstpress = true:
						unsigned long currentMillis = millis()
						if (currentMillis - previusMillis) >= 3 segundos:
							previusMillis = currentMillis
							firstpress == false*
		Con este bloque tendria que funcionar el chequeo para second-tap overflow.
	**Solucion:**
		*Se uso el parametro startCount, y se agrego la funcion Overflow para chequear el estado del tiempo transcurrido desde el primer tap:
			void Overflow(int timeout)
			{
			  if (firstPress)
			  {
			    unsigned long currentMillis_OF = millis();
			    if ((currentMillis_OF - startCount) >= timeout)
			    {
			      firstPress = false;
			    }
			  }
			}*
6. Detectar switch apretado durante mas de x segundos: FALTA
	main loop:
		botonState = digitalRead(switch)
		if botonState = true:
			unsigned long currentMillis = millis():
			if (currentMillis - previusMillis) >= x segundos:
				previousMillis = currentMillis
				ejecutar_funcion

7. Tape modulation: EN DESARROLLO
	Investigar como funciona la modulacion por cinta.
	Se puede emular las modulaciones debido a las partes moviles del mecanismo de la maquina de eco o a la variacion de la velocidad del motor. Esto se podria hacer utilizando valores random en un rango muy limitado de delay (no mas de 20-40 ms?), apareciendo con una recurrencia tambien random. Es decir:
	 	if (tape_modulation_set == true):
	 		unsigned long currentTime = millis;
	 		if (currentTime - previusTime) >= random(recurrencia):
	 			previusMillis = currentMillis
	 			digiPotWrite((tappedTempo + random(-20, 20))) 


8. Infinite Feedback con tap hold:
