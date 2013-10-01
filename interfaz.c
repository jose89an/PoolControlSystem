void menu(){
   /*
      Una vez haya saltado la interrupción externa correspondiente, se activará un flag que hará que en el bucle de ejecución del programa
      se dé paso a este método. El menú gestionará qué opción está siendo accedida en ese momento según se haya seleccionado con los botones,
      así como el acceso a las distintas opciones (dando paso a otros métodos) cuando así se seleccione. Además, el menú se cerrará completamente
      al pulsar el botón de salida y pondrá a 0 el flag correspondiente.
   */
   int opcion=0;     // Esta variable determina en qué opción del menú me encuentro

   setup_wdt(WDT_OFF);  // Desactivaremos el perro guardián en el menú para que no reinicie al PIC
   // Se muestra por el LCD el mensaje correspondiente al menú, y muestra la primera opción
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   lcd_putc("MENU DEL SISTEMA");
   lcd_gotoxy(1,2);
   lcd_putc("Depuradora      ");
   while(!input(PIN_B1));  // Espero hasta que el botón de acceso al menú no esté pulsado
   while(1){

      if (!input(PIN_E1))  // En caso que se pulse el botón de salida, se sale del bucle
         break;

      if (!input(PIN_A4)){ // En caso de que se pulse el botón de siguiente
         while(!input(PIN_A4));  // Espero hasta que el botón no esté pulsado
         opcion++;   // Avanzo a la siguiente opción
         lcd_gotoxy(1,2);  // Me voy a la segunda línea del LCD
         switch(opcion){
            // Muestro un mensaje u otro según la opción que haya seleccionada
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
         while(!input(PIN_E0));  // Espero hasta que el botón no esté pulsado
            switch(opcion){
               // Se accede a un menú u otro según se haya seleccionado
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
   // Se vuelve a activar al perro guardián
   setup_wdt(WDT_ON);
   // Comprueba la temperatura del agua y muestra datos por el LCD justo al salir del menú
   comprobar_temperatura();
   mostrar_lcd();
   mostrar_rs232();
}


void menuAlarma(){
   /*
      En este menú se harán las gestiones relativas a la hora de encendido y apagado de la depuradora, pudiendo consultarse y modificarse.
      Será importante que estos valores modificados se guarden en la EEPROM, y que se guarde en el PCF8583 la alarma correspondiente si es
      la próxima que debería utilizar. Por ejemplo, si la depuradora está apagada y se cambia la hora de inicio, deberá introducirse esta
      hora en el RTC.
   */
   int inicfin=0;       // Flag para alternar si estamos en la opción de hora de inicio o de fin
   int num1,num2,num3,num4;   // Números que se introducirán con el teclado matricial
   int ntp=0;           // Número de teclas pulsadas en el teclado matricial hasta el momento
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   // Empieza mostrando la opción de variar la hora de inicio
   printf(lcd_putc,"Hora in.: %u:%u",horaInicio,minutoInicio);
   lcd_gotoxy(1,2);
   lcd_putc("Modificar?");
   while(1){
      if (!input(PIN_A4)){ // Cuando se pulsa el botón de siguiente
         while(!input(PIN_A4));
         // Se muestra un mensaje u otro según estemos en la opción de hora de inicio o de fin
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

      if (!input(PIN_E1)){ // Cuando se pulsa el botón de salir, se vuelve al menú anterior
         while(!input(PIN_E1));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("MENU DEL SISTEMA");
         lcd_gotoxy(1,2);
         lcd_putc("Depuradora      ");
         break;   // Volverá al bucle de la opción menu()
      }

      if (!input(PIN_E0) && ntp!=4){
         // Cuando se pulse el botón de aceptar y no se hayan introducido ya los 4 dígitos, mostrará la opción de modificar la hora
         while(!input(PIN_E0));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         if (inicfin==0)   // Según sea hora de inicio o de fin
            lcd_putc("Nueva hora inic.");
         else
            lcd_putc("Nueva hora fin");
         lcd_gotoxy(1,2);
         lcd_putc("Formato: XX:XX");
         lcd_gotoxy(10,2);
         while(ntp<4){
            // Bucle de ejecución mientras se introducen los números

            // Si se pulsa el botón de salir, vuelve a la opción anterior
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

            // Según las teclas que haya pulsado (iré por un dígito u otro de las horas:minutos)
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
         // Compruebo que la hora está entre 0 y 23
         if((10*num1+num2)<24){
            if (inicfin==0){  // Si es la opción de modificar hora de inicio
               horaInicio=10*num1+num2;
               minutoInicio=10*num3+num4; // La guardamos en sus variables correspondientes
               if (depuradoraOn==0){   // Además, si la depuradora está apagada, introduzco esta hora de inicio en el RTC
                  dtalarm.hours=horaInicio;
                  dtalarm.minutes=minutoInicio;
                  PCF8583_set_alarmtime(&dtalarm);
               }
            }
            else{ // Si es la opción de modificar hora de fin
               horaFin=10*num1+num2;
               minutoFin=10*num3+num4;
               if (depuradoraOn==1){   // Esta vez si la depuradora está encendida, introduzco la hora de fin
                  dtalarm.hours=horaFin;
                  dtalarm.minutes=minutoFin;
                  PCF8583_set_alarmtime(&dtalarm);
               }
            }
            delay_ms(10);
            escribeEeprom();
         }
         ntp=0;   // El número de teclas pulsadas vuelve a cero
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
      En este menú simplemente modificamos la temperatura a la que deseamos que se encuentre el agua en la piscina. Será una variable
      flotante, pero la definiremos con 2 dígitos de parte entera y 1 de parte decimal.
   */
   int num1,num2,num3;  // Los 3 dígitos del número flotante (parte entera y decimal)
   int ntp=0;  // Número de teclas pulsadas
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Temp. des.: %2.1f",tempDeseada);
   lcd_gotoxy(1,2);
   lcd_putc("Modificar?");
   while(1){
      if (!input(PIN_E1)){ // En caso de que se pulse salir, vuelve al menú principal (y lo deja por la opción de temperatura)
         while(!input(PIN_E1));
         lcd_send_byte(0,1);
         lcd_gotoxy(1,1);
         lcd_putc("MENU DEL SISTEMA");
         lcd_gotoxy(1,2);
         lcd_putc("Temperatura agua");
         break;
      }
      // En caso de que se pulse aceptar antes de que se hayan introducido los 3 dígitos
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
            // En caso de que se pulse salir, volvemos a la opción anterior
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
               // Según las teclas pulsadas
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
      El funcionamiento de este menú será idéntico al definido para la temperatura, pero únicamente con un dígito entero y otro decimal
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
      Este menú es idéntico al de modificar el nivel de cloro
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
