void menu(){
   /*
      Una vez haya saltado la interrupci�n externa correspondiente, se activar� un flag que har� que en el bucle de ejecuci�n del programa
      se d� paso a este m�todo. El men� gestionar� qu� opci�n est� siendo accedida en ese momento seg�n se haya seleccionado con los botones,
      as� como el acceso a las distintas opciones (dando paso a otros m�todos) cuando as� se seleccione. Adem�s, el men� se cerrar� completamente
      al pulsar el bot�n de salida y pondr� a 0 el flag correspondiente.
   */
   int opcion=0;     // Esta variable determina en qu� opci�n del men� me encuentro

   setup_wdt(WDT_OFF);  // Desactivaremos el perro guardi�n en el men� para que no reinicie al PIC
   // Se muestra por el LCD el mensaje correspondiente al men�, y muestra la primera opci�n
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   lcd_putc("MENU DEL SISTEMA");
   lcd_gotoxy(1,2);
   lcd_putc("Depuradora      ");
   while(!input(PIN_B1));  // Espero hasta que el bot�n de acceso al men� no est� pulsado
   while(1){

      if (!input(PIN_E1))  // En caso que se pulse el bot�n de salida, se sale del bucle
         break;

      if (!input(PIN_A4)){ // En caso de que se pulse el bot�n de siguiente
         while(!input(PIN_A4));  // Espero hasta que el bot�n no est� pulsado
         opcion++;   // Avanzo a la siguiente opci�n
         lcd_gotoxy(1,2);  // Me voy a la segunda l�nea del LCD
         switch(opcion){
            // Muestro un mensaje u otro seg�n la opci�n que haya seleccionada
            case 1: lcd_putc("Temperatura agua");
            break;
            case 2: lcd_putc("Nivel cloro     ");
            break;
            case 3: lcd_putc("Nivel turbidez  ");
            break;
            case 4: lcd_putc("Depuradora      ");
            opcion=0;
            break;
         }
      }

      if (!input(PIN_E0)){    // En caso de que se pulse aceptar
         while(!input(PIN_E0));  // Espero hasta que el bot�n no est� pulsado
            switch(opcion){
               // Se accede a un men� u otro seg�n se haya seleccionado
               case 0: menuAlarma();
               break;
               case 1: menuTemperatura();
               break;
               case 2: menuCloro();
               break;
               case 3: menuTurbidez();
               break;
            }
      }
   comprobar_temperatura();
   if (flagDepuradora==1){
         funcionaDepuradora();
      }
   }
   // El flag vuelve a 0 cuando se ha salido del bucle
   flag=0;
   // Se vuelve a activar al perro guardi�n
   setup_wdt(WDT_ON);
   // Comprueba la temperatura del agua y muestra datos por el LCD justo al salir del men�
   comprobar_temperatura();
   mostrar_lcd();
   mostrar_rs232();
}


void menuAlarma(){
   /*
      En este men� se har�n las gestiones relativas a la hora de encendido y apagado de la depuradora, pudiendo consultarse y modificarse.
      Ser� importante que estos valores modificados se guarden en la EEPROM, y que se guarde en el PCF8583 la alarma correspondiente si es
      la pr�xima que deber�a utilizar. Por ejemplo, si la depuradora est� apagada y se cambia la hora de inicio, deber� introducirse esta
      hora en el RTC.
   */
   int inicfin=0;       // Flag para alternar si estamos en la opci�n de hora de inicio o de fin
   int num1,num2,num3,num4;   // N�meros que se introducir�n con el teclado matricial
   int ntp=0;           // N�mero de teclas pulsadas en el teclado matricial hasta el momento
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   // Empieza mostrando la opci�n de variar la hora de inicio
   printf(lcd_putc,"Hora in.: %u:%u",horaInicio,minutoInicio);
   lcd_gotoxy(1,2);
   lcd_putc("Modificar?");
   while(1){
      if (!input(PIN_A4)){ // Cuando se pulsa el bot�n de siguiente
         while(!input(PIN_A4));
         // Se muestra un mensaje u otro seg�n estemos en la opci�n de hora de inicio o de fin
         if (inicfin==0){
            inicfin=1;
            lcd_send_byte(0,1);
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Hora fin: %u:%u",horaFin,minutoFin);
            lcd_gotoxy(1,2);
            lcd_putc("Modificar?");
         }
         else{
            inicfin=0;
            lcd_send_byte(0,1);
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Hora in.: %u:%u",horaInicio,minutoInicio);
            lcd_gotoxy(1,2);
            lcd_putc("Modificar?");
         }
      }

      if (!input(PIN_E1)){ // Cuando se pulsa el bot�n de salir, se vuelve al men� anterior
         while(!input(PIN_E1));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("MENU DEL SISTEMA");
         lcd_gotoxy(1,2);
         lcd_putc("Depuradora      ");
         break;   // Volver� al bucle de la opci�n menu()
      }

      if (!input(PIN_E0) && ntp!=4){
         // Cuando se pulse el bot�n de aceptar y no se hayan introducido ya los 4 d�gitos, mostrar� la opci�n de modificar la hora
         while(!input(PIN_E0));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         if (inicfin==0)   // Seg�n sea hora de inicio o de fin
            lcd_putc("Nueva hora inic.");
         else
            lcd_putc("Nueva hora fin");
         lcd_gotoxy(1,2);
         lcd_putc("Formato: XX:XX");
         lcd_gotoxy(10,2);
         while(ntp<4){
            // Bucle de ejecuci�n mientras se introducen los n�meros

            // Si se pulsa el bot�n de salir, vuelve a la opci�n anterior
            if (!input(PIN_E1)){
            while(!input(PIN_E1));
            lcd_send_byte(0,1);
            lcd_gotoxy(1,1);
            if (inicfin==0)
               printf(lcd_putc,"Hora in.: %u:%u",horaInicio,minutoInicio);
            else
               printf(lcd_putc,"Hora fin: %u:%u",horaFin,minutoFin);
            lcd_gotoxy(1,2);
            lcd_putc("Modificar?");
            ntp=0;
            break;
            }

            // Seg�n las teclas que haya pulsado (ir� por un d�gito u otro de las horas:minutos)
            switch(ntp){
               case 0:  num1=kbd_getc();
               if (num1<=2){
                  printf(lcd_putc,"%u",num1);
                  ntp++;
               }
               break;
               case 1:  num2=kbd_getc();
               if (num2<10){
                  printf(lcd_putc,"%u:",num2);
                  ntp++;
               }
               break;
               case 2:  num3=kbd_getc();
               if (num3<=5){
                  printf(lcd_putc,"%u",num3);
                  ntp++;
               }
               break;
               case 3:  num4=kbd_getc();
               if (num4<10){
                  printf(lcd_putc,"%u",num4);
                  ntp++;
               }
               break;
            }
         comprobar_temperatura();
         if (flagDepuradora==1)
            funcionaDepuradora();
         }
      }
      // En el caso de que pulse aceptar y ya haya introducido la hora completa
      if (!input(PIN_E0) && ntp==4){
         while(!input(PIN_E0));
         // Compruebo que la hora est� entre 0 y 23
         if((10*num1+num2)<24){
            if (inicfin==0){  // Si es la opci�n de modificar hora de inicio
               horaInicio=10*num1+num2;
               minutoInicio=10*num3+num4; // La guardamos en sus variables correspondientes
               if (depuradoraOn==0){   // Adem�s, si la depuradora est� apagada, introduzco esta hora de inicio en el RTC
                  dtalarm.hours=horaInicio;
                  dtalarm.minutes=minutoInicio;
                  PCF8583_set_alarmtime(&dtalarm);
               }
            }
            else{ // Si es la opci�n de modificar hora de fin
               horaFin=10*num1+num2;
               minutoFin=10*num3+num4;
               if (depuradoraOn==1){   // Esta vez si la depuradora est� encendida, introduzco la hora de fin
                  dtalarm.hours=horaFin;
                  dtalarm.minutes=minutoFin;
                  PCF8583_set_alarmtime(&dtalarm);
               }
            }
            delay_ms(10);
            escribeEeprom();
         }
         ntp=0;   // El n�mero de teclas pulsadas vuelve a cero
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         // Muestro el mensaje anterior de nuevo
         if (inicfin==0)
            printf(lcd_putc,"Hora in.: %u:%u",horaInicio,minutoInicio);
         else
            printf(lcd_putc,"Hora fin: %u:%u",horaFin,minutoFin);
         lcd_gotoxy(1,2);
         lcd_putc("Modificar?");
      }
   comprobar_temperatura();
   if (flagDepuradora==1)
      funcionaDepuradora();
   }
}

void menuTemperatura(){
   /*
      En este men� simplemente modificamos la temperatura a la que deseamos que se encuentre el agua en la piscina. Ser� una variable
      flotante, pero la definiremos con 2 d�gitos de parte entera y 1 de parte decimal.
   */
   int num1,num2,num3;  // Los 3 d�gitos del n�mero flotante (parte entera y decimal)
   int ntp=0;  // N�mero de teclas pulsadas
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Temp. des.: %2.1f",tempDeseada);
   lcd_gotoxy(1,2);
   lcd_putc("Modificar?");
   while(1){
      if (!input(PIN_E1)){ // En caso de que se pulse salir, vuelve al men� principal (y lo deja por la opci�n de temperatura)
         while(!input(PIN_E1));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("MENU DEL SISTEMA");
         lcd_gotoxy(1,2);
         lcd_putc("Temperatura agua");
         break;
      }
      // En caso de que se pulse aceptar antes de que se hayan introducido los 3 d�gitos
      if (!input(PIN_E0) && ntp!=3){
         while(!input(PIN_E0));
         // Se accede a modificar la temperatura
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("Introduzca temp.");
         lcd_gotoxy(1,2);
         lcd_putc("Formato: XX.X");
         lcd_gotoxy(10,2);
         while(ntp<3){
            // Esto se ejecuta mientras no haya introducido la nueva temperatura
            if (!input(PIN_E1)){
            // En caso de que se pulse salir, volvemos a la opci�n anterior
            while(!input(PIN_E1));
            lcd_send_byte(0,1);
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Temp. des.: %2.1f",tempDeseada);
            lcd_gotoxy(1,2);
            lcd_putc("Modificar?");
            ntp=0;
            break;
            }
            switch(ntp){
               // Seg�n las teclas pulsadas
               case 0:  num1=kbd_getc();
               if (num1<10){
                  printf(lcd_putc,"%u",num1);
                  ntp++;
               }
               break;
               case 1:  num2=kbd_getc();
               if (num2<10){
                  printf(lcd_putc,"%u.",num2);
                  ntp++;
               }
               break;
               case 2:  num3=kbd_getc();
               if (num3<10){
                  printf(lcd_putc,"%u",num3);
                  ntp++;
               }
               break;
            }
         comprobar_temperatura();
         if (flagDepuradora==1)
            funcionaDepuradora();
         }
      }
      if (!input(PIN_E0) && ntp==3){
         // Una vez se introduce la temperatura y se pulsa aceptar
         while(!input(PIN_E0));
         // Introducimos la temperatura en una variable tipo float
         tempDeseada=10*num1+num2+0.1*num3;
         delay_ms(10);
         escribeEeprom();
         ntp=0;
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         printf(lcd_putc,"Temp. des.: %2.1f",tempDeseada);
         lcd_gotoxy(1,2);
         lcd_putc("Modificar?");
      }
   comprobar_temperatura();
   if (flagDepuradora==1)
      funcionaDepuradora();
   }
}

void menuCloro(){
   /*
      El funcionamiento de este men� ser� id�ntico al definido para la temperatura, pero �nicamente con un d�gito entero y otro decimal
   */
   int num1,num2;
   int ntp=0;
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Cloro. des.: %1.1f",cloroDeseado);
   lcd_gotoxy(1,2);
   lcd_putc("Modificar?");
   while(1){
      if (!input(PIN_E1)){
         while(!input(PIN_E1));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("MENU DEL SISTEMA");
         lcd_gotoxy(1,2);
         lcd_putc("Nivel cloro     ");
         break;
      }

      if (!input(PIN_E0) && ntp!=2){
         while(!input(PIN_E0));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("Nuevo nv. cloro");
         lcd_gotoxy(1,2);
         lcd_putc("Formato: X.X");
         lcd_gotoxy(10,2);
         while(ntp<2){
            if (!input(PIN_E1)){
            while(!input(PIN_E1));
            lcd_send_byte(0,1);
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Cloro. des.: %1.1f",cloroDeseado);
            lcd_gotoxy(1,2);
            lcd_putc("Modificar?");
            ntp=0;
            break;
            }
            switch(ntp){
               case 0:  num1=kbd_getc();
               if (num1<10){
                  printf(lcd_putc,"%u.",num1);
                  ntp++;
               }
               break;
               case 1:  num2=kbd_getc();
               if (num2<10){
                  printf(lcd_putc,"%u",num2);
                  ntp++;
               }
               break;
            }
         comprobar_temperatura();
         if (flagDepuradora==1)
            funcionaDepuradora();
         }
      }
      if (!input(PIN_E0) && ntp==2){
         while(!input(PIN_E0));
         if(num1<5 || (num1==5 && num2==0)){
            cloroDeseado=num1+0.1*num2;
            delay_ms(10);
            escribeEeprom();
         }
         ntp=0;
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         printf(lcd_putc,"Cloro. des.: %1.1f",cloroDeseado);
         lcd_gotoxy(1,2);
         lcd_putc("Modificar?");
      }
   comprobar_temperatura();
   if (flagDepuradora==1)
      funcionaDepuradora();
   }
}

void menuTurbidez(){
   /*
      Este men� es id�ntico al de modificar el nivel de cloro
   */
   int num1,num2;
   int ntp=0;
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Turbi. des.: %1.1f",turbidezDeseada);
   lcd_gotoxy(1,2);
   lcd_putc("Modificar?");
   while(1){
      if (!input(PIN_E1)){
         while(!input(PIN_E1));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("MENU DEL SISTEMA");
         lcd_gotoxy(1,2);
         lcd_putc("Nivel turbidez  ");
         break;
      }

      if (!input(PIN_E0) && ntp!=2){
         while(!input(PIN_E0));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("Nuevo nv. turb.");
         lcd_gotoxy(1,2);
         lcd_putc("Formato: X.X");
         lcd_gotoxy(10,2);
         while(ntp<2){
            if (!input(PIN_E1)){
            while(!input(PIN_E1));
            lcd_send_byte(0,1);
            lcd_gotoxy(1,1);
            printf(lcd_putc,"Turbi. des.: %1.1f",turbidezDeseada);
            lcd_gotoxy(1,2);
            lcd_putc("Modificar?");
            ntp=0;
            break;
            }
            switch(ntp){
               case 0:  num1=kbd_getc();
               if (num1<10){
                  printf(lcd_putc,"%u.",num1);
                  ntp++;
               }
               break;
               case 1:  num2=kbd_getc();
               if (num2<10){
                  printf(lcd_putc,"%u",num2);
                  ntp++;
               }
               break;
            }
         comprobar_temperatura();
         if (flagDepuradora==1)
            funcionaDepuradora();
         }
      }
      if (!input(PIN_E0) && ntp==2){
         while(!input(PIN_E0));
         if(num1<5 || (num1==5 && num2==0)){
            turbidezDeseada=num1+0.1*num2;
            delay_ms(10);
            escribeEeprom();
         }
         ntp=0;
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         printf(lcd_putc,"Turbi. des.: %1.1f",turbidezDeseada);
         lcd_gotoxy(1,2);
         lcd_putc("Modificar?");
      }
   comprobar_temperatura();
   if (flagDepuradora==1)
      funcionaDepuradora();
   }
}
