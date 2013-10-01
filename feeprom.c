void escribeEeprom(){
   /*
      Este método escribirá ciertos datos referidos a la configuración de la piscina en memoria EEPROM, tanto en la interna del PIC
      como en una memoria I2C externa incluida en el modelo de Proteus. Estos datos a guardar serán 4 enteros (hora y minuto tanto
      del encendido como del apagado de la depuradora) y 3 flotantes (valores deseados de temperatura, cloro y turbidez). Los flotantes
      los guardaremos en 4 posiciones de memoria al ser de 32 bits, haciendo esta división con ayuda de un puntero que señale al inicio
      de dónde están almacenados los flotantes en memoria. Se comenzará la escritura con un flag de inicio que determinará si hemos
      escrito o no en la memoria para luego reconocer estos datos en lecturas sucesivas.
      Se usará este método cada vez que el usuario modifique en el menú los datos relativos al funcionamiento del sistema.
   */
   int ptd,pcd,ptbd; // Punteros para las variables flotantes, serán de tipo int que son 8 bits, justo como en las posiciones de memoria
   ptd=&tempDeseada;
   pcd=&cloroDeseado;
   ptbd=&turbidezDeseada;
   // Empezamos con un flag para reconocer que hemos escrito en memoria
   write_eeprom(0x9E,0xAB);
   write_eeprom(0x9F,0xCD);
   // Empezamos a escribir en la posición A0 de la memoria eeprom
   write_eeprom(0xA0,horaInicio);
   write_eeprom(0xA1,minutoInicio);
   write_eeprom(0xA2,horaFin);
   write_eeprom(0xA3,minutoFin);
   write_eeprom(0xA4,*ptd);
   write_eeprom(0xA5,*(ptd+1));
   write_eeprom(0xA6,*(ptd+2));
   write_eeprom(0xA7,*(ptd+3));
   write_eeprom(0xA8,*pcd);
   write_eeprom(0xA9,*(pcd+1));
   write_eeprom(0xAA,*(pcd+2));
   write_eeprom(0xAB,*(pcd+3));
   write_eeprom(0xAC,*ptbd);
   write_eeprom(0xAD,*(ptbd+1));
   write_eeprom(0xAE,*(ptbd+2));
   write_eeprom(0xAF,*(ptbd+3));

   // Ahora vamos a la memoria I2C
   i2c_start();
   i2c_write(0xAE);  // Esto es AE porque este dispositivo I2C (la eeprom) tiene como dirección 1 0 1 0 A2 A1 A0 0
                     // Y como A2, A1 y A0 en el Proteus los he conectado a +5V, estos valdrán 1, por lo que la secuencia será 1010 1110,
                     // o lo que es lo mismo, AE. Esto lo hago así para que la dirección de la memoria externa EEPROM no sea la misma que la
                     // del PCF8583, que es también un dispositivo I2C y su dirección es A0, que sería la de este caso si A2, A1 y A0 fueran a tierra.
   i2c_write(0x00);  // Posición inicial de memoria -> 00A0
   i2c_write(0xA0);
   i2c_write(horaInicio);
   i2c_write(minutoInicio);
   i2c_write(horaFin);
   i2c_write(minutoFin);
   i2c_write(*ptd);
   i2c_write(*(ptd+1));
   i2c_write(*(ptd+2));
   i2c_write(*(ptd+3));
   i2c_write(*pcd);
   i2c_write(*(pcd+1));
   i2c_write(*(pcd+2));
   i2c_write(*(pcd+3));
   i2c_write(*ptbd);
   i2c_write(*(ptbd+1));
   i2c_write(*(ptbd+2));
   i2c_write(*(ptbd+3));
   i2c_stop();
   delay_ms(50);
   
   // Inicializo otra vez I2C ahora para escribir el flag (no me deja escribirlo todo seguido)
   i2c_start();
   i2c_write(0xAE);
   i2c_write(0x00);
   i2c_write(0x9E);  // Ahora escribiremos el flag en la posición de memoria 0x9E y 0x9F
   i2c_write(0xAB);
   i2c_write(0xCD);
   i2c_stop();
}

void leeEeprom(){
   /*
      Este método lee los datos guardados en eeprom y los carga en las variables de control del sistema. El método normalmente tratará
      de hacer la lectura en la memoria interna, pero si no encuentra datos introducidos por el usuario (ejemplo, ha habido que cambiar
      el PIC por fallo de funcionamiento), pasará a buscar los datos en la memoria I2C externa, que debería haber quedado intacta. En
      caso de no encontrar datos introducidos por el usuario en ninguna de las dos memorias, el sistema cargará unos valores predefinidos.
      El método se lanzará cada vez que se inicie el funcionamiento del sistema.
   */
   int ptd, pcd, ptbd, flag1, flag2, flag3, flag4;
   // Punteros para las variables flotantes
   ptd=&tempDeseada;
   pcd=&cloroDeseado;
   ptbd=&turbidezDeseada;
   // Lee donde deberían haberse guardado los flags en la memoria interna
   flag1=read_eeprom(0x9E);
   flag2=read_eeprom(0x9F);
   
   // Leemos en I2C primero para obtener los flags
   i2c_start();
   i2c_write(0xAE);     // Byte de control de escritura.
   i2c_write(0x00);     // 2 bytes de dirección
   i2c_write(0x9E);
   i2c_start();
   i2c_write(0xAF);           // Byte de control de lectura.
   // Lee donde deberían haberse guardado los flags en la memoria externa
   flag3=i2c_read(1);
   flag4=i2c_read(0);
   i2c_stop();

   // Comprobamos si encontramos el flag de haber escrito en la eeprom
   if (flag1==0xAB && flag2==0xCD){  // En este caso leo de la memoria interna
      horaInicio=read_eeprom(0xA0);
      minutoInicio=read_eeprom(0xA1);
      horaFin=read_eeprom(0xA2);
      minutoFin=read_eeprom(0xA3);
      *ptd=read_eeprom(0xA4);
      *(ptd+1)=read_eeprom(0xA5);
      *(ptd+2)=read_eeprom(0xA6);
      *(ptd+3)=read_eeprom(0xA7);
      *pcd=read_eeprom(0xA8);
      *(pcd+1)=read_eeprom(0xA9);
      *(pcd+2)=read_eeprom(0xAA);
      *(pcd+3)=read_eeprom(0xAB);
      *ptbd=read_eeprom(0xAC);
      *(ptbd+1)=read_eeprom(0xAD);
      *(ptbd+2)=read_eeprom(0xAE);
      *(ptbd+3)=read_eeprom(0xAF);
   }
   else if(flag3==0xAB && flag4==0xCD){    // En este otro caso leeré de la memoria I2C externa
      i2c_start();
      i2c_write(0xAE);     // Byte de control de escritura.
      i2c_write(0x00);     // 2 bytes de dirección
      i2c_write(0xA0);

      i2c_start();
      i2c_write(0xAF);           // Byte de control de lectura.
      horaInicio=i2c_read(1);    // Con ACK, indicando que hay más datos a leer.
      minutoInicio=i2c_read(1);
      horaFin=i2c_read(1);
      minutoFin=i2c_read(1);
      *ptd=i2c_read(1);
      *(ptd+1)=i2c_read(1);
      *(ptd+2)=i2c_read(1);
      *(ptd+3)=i2c_read(1);
      *pcd=i2c_read(1);
      *(pcd+1)=i2c_read(1);
      *(pcd+2)=i2c_read(1);
      *(pcd+3)=i2c_read(1);
      *ptbd=i2c_read(1);
      *(ptbd+1)=i2c_read(1);
      *(ptbd+2)=i2c_read(1);
      *(ptbd+3)=i2c_read(0);     // Último dato a leer. ACK=0.
      i2c_stop();
   }
   else{
      // En otro caso (no se encuentran los flags), cargamos unos valores predeterminados
      horaInicio=19;
      minutoInicio=30;
      horaFin=20;
      minutoFin=30;
      tempDeseada=25.0;
      cloroDeseado=4.0;
      turbidezDeseada=1.0;
   }
   // Al leer los datos de la memoria EEPROM y haberlos asignados en las variables correspondientes, será de especial interés
   // lo relativo a la hora de encendido y apagado de la depuradora. Comprobamos si está encendida o apagada para introducir
   // en el PCF8583 una hora u otra según esté o no apagada la depuradora.
   if (depuradoraOn==0){
      // Si no está funcionando, metemos en el PCF la hora de inicio del ciclo. Será lo usual si se supone que leemos en eeprom
      // al inicializar el PIC (depuradora apagada), pero para contemplar posibles cambios en el funcionamiento haremos la
      // comprobación
      dtalarm.hours=horaInicio;
      dtalarm.minutes=minutoInicio;
      PCF8583_set_alarmtime(&dtalarm);
   }
   else{
      // En caso de estar activa, se introduce la hora de fin del ciclo
      dtalarm.hours=horaFin;
      dtalarm.minutes=minutoFin;
      PCF8583_set_alarmtime(&dtalarm);
   }
}
