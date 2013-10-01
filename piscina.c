#include "18F4520.H"
#fuses HS, WDT, WDT32768, NOPROTECT, BROWNOUT, PUT, NOLVP, NOXINST
#use delay(clock=8000000, restart_wdt)
#use rs232(baud=4800, xmit=PIN_C6, rcv=PIN_C7, stream=FT232)
#include <ctype.h>
#include "PCF8583.c"
#include "LCDeasy.c"
#include "1wire.c"
#include "ds1820.c"
#include "kbd2.c"
#include "datos_funciones.c"  // Los datos usados en el proyecto así como las funciones serán definidas previamente en este archivo
#include "feeprom.c"          // Funciones relativas a lectura y escritura en eeprom
#include "interfaz.c"         // Todas las funciones relativas a la gestión del menú de usuario
#use fast_io(A)
#use fast_io(B)


#int_TIMER1
void manejo_timer1(){
   /*
      El timer 1 está configurado para trabajar con un prescaler de 8, y le asignamos 15536 cuentas de inicio, por lo que se desbordará
      cada 0.2 segundos, el tiempo equivalente a dosificar un valor de 0.1 voltios en cloro o floculante. Esto se hace así porque los
      niveles deseados de cloro y turbidez solo tendrán una precisión de un decimal al definirlos en el menú
      Únicamente se harán las comprobaciones de cloro y turbidez cuando se acabe el ciclo de encendido de la depuradora. Mediante el método
      dosificadores será la única forma de que las variables difCloro y difTurbidez tomen un valor superior a 0. Esto ocurrirá cuando sea
      necesario dosificar cloro o floculante, y será cuando se reste cloro en una cantidad proporcional al tiempo de desbordamiento del
      timer 1.
   */
   // Asignamos 15536 cuentas al timer 1, para que así se desborde tras 50000 cuentas (0.2 segundos)
   set_timer1(15536);

   if (difCloro>0.0){
      // Resta 0.1 a la diferencia de cloro (para que así hayan pasado 0.2 segundos) y pone el pin a 1
      difCloro-=0.1;
      output_high(PIN_C1);
   }
   else{
      // Una vez no haya que dosificar más, se pone el pin a 0
      output_low(PIN_C1);
   }
   // Se opera de la misma forma con la turbidez
   if (difTurbidez>0.0){
      difTurbidez-=0.1;
      output_high(PIN_C2);
   }
   else{
      output_low(PIN_C2);
   }
}

#int_EXT
void manejo_ext(){
   /*
      La interrupción externa saltará cada vez que se inicie o termine el ciclo de funcionamiento de la depuradora. Es decir, cuando salte
      una alarma en el PCF8583. Dependiendo de un flag que indica si la depuradora está o no encendida, se pondrá en marcha o se detendrá
      el motor de la depuradora.
   */
   flagDepuradora=1;
}

#int_EXT1
void manejo_ext1(){
   // Esta interrupción externa saltará cuando se quiera acceder al menú de usuario
   flag=1;  // Simplemente se activa un flag que indica que se quiere acceder al menú
}



void mostrar_lcd(){
   /*
      Este método mostrará por la pantalla del LCD varios valores de control de la piscina. Concretamente, iniciará mostrando la cantidad
      de cloro y turbidez entre 0 y 5 voltios que ha medido, así como la temperatura del agua y la hora en la que la ha medida. Serán estos
      últimos valores los que se quedarán en el LCD cada vez que el PIC se vaya a dormir.
   */
   float temperature, cloro, turbidez;          // Variables de los datos medidos en este momento
   lcd_send_byte(0,2);
   PCF8583_read_datetime(&dt);                  // Lectura del RTC
   temperature = ds1820_read();                 // Lectura de temperatura
   strcpy(weekday, weekday_names[dt.weekday]);  // ptr=strncpy (s1, s2,  n)
                                                // Copy up to n characters s2->s1
   // Seleccionamos el canal analógico correspondiente al cloro y lo leemos
   set_adc_channel(0);
   delay_us(10);
   cloro=read_adc();
   cloro=cloro*5/255;         // Normalizamos el valor entre 0 y 5
   // Ahora se hará lo mismo con la turbidez
   set_adc_channel(1);
   delay_us(10);
   turbidez=read_adc();
   turbidez=turbidez*5/255;
   // Se muestran los datos por el LCD
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"CLORO: %3.2f \nTURBIDEZ: %3.2f ",cloro,turbidez);
   delay_ms(1000);
   // Espera un segundo tras mostrar cloro y turbidez para luego mostrar hora y fecha
   lcd_send_byte(0,1);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"TEMP: %3.1f %cC \nHora: %u:%2u:%2u",temperature,223,dt.hours, dt.minutes,dt.seconds );
   delay_ms(1000);
}

void mostrar_rs232(){
   leeEeprom();
   printf("Lectura de Eeprom hecha a las %u:%2u:%2u\n\r\n\r",dt.hours, dt.minutes,dt.seconds);
   printf("Hora inicio ciclo depuradora: %u:%u\n\r",horaInicio,minutoInicio);
   printf("Hora fin ciclo depuradora: %u:%u\n\r",horaFin,minutoFin);
   printf("Temperatura minima deseada: %2.1f\n\r",tempDeseada);
   printf("Nivel minimo de cloro exigido: %1.1f\n\r",cloroDeseado);
   printf("Nivel maximo de turbidez permitida: %1.1f\n\r",turbidezDeseada);
   printf("------------------------------------\n\r");
}

void comprobar_temperatura(){
   /*
      Este método hará una medición de la temperatura y además comprobará si debe o no encender el calentador del
      agua. Se permite un margen de 2 grados centígrados respecto el valor deseado antes de encender el calentador.
      El método saltará cada vez que el PIC despierte, normalmente avisado por el wdt, cada algo más de 2 minutos.
   */
   float temperature;
   temperature = ds1820_read();
   if (temperature <= tempdeseada - 2.0){    // Damos histéresis de 2ºC
      output_high(PIN_C0);
   }
   else{
      output_low(PIN_C0);
   }
}

void dosificadores(){
   /*
      Este método se lanzará cada vez que acabe el ciclo de trabajo de la depuradora. La función del método será leer
      los valores de cloro y turbidez que hay en la piscina en ese momento y compararlos con los valores deseados
      definidos por el usuario. En caso de necesitar dosificarse cloro o floculante, las variables difCloro y difTurbidez
      tendrán un valor mayor que 0, y por tanto la rutina del timer 1 empezaría a dosificar durante un tiempo proporcional
      a esta diferencia, que se irá reduciendo con sucesivas llamadas a la rutina de interrupción del timer 1.
   */
   float cloro, turbidez;
   // Medimos los niveles actuales de cloro y turbidez
   set_adc_channel(0);
   delay_us(10);
   cloro=read_adc();
   cloro=cloro*5/255;
   set_adc_channel(1);
   delay_us(10);
   turbidez=read_adc();
   turbidez=turbidez*5/255;
   // Calculamos la diferencia con los valores deseados
   difCloro=cloroDeseado-cloro;              // En el caso del cloro, la diferencia es positiva cuando hay menos cloro del deseado
   difTurbidez=turbidez-turbidezDeseada;     // En el caso de la turbidez, habrá que dosificar cuando hay más turbidez de la deseada
   // En ambos casos, diferencias negativas (que los niveles sean mejores que los deseados) serán tomadas como cero.
   if (difCloro<0.0)
      difCloro=0.0;
   if (difTurbidez<0.0)
      difTurbidez=0.0;
}

void funcionaDepuradora(){
   /*
      Este método será el que determine el funcionamiento de la depuradora, con su ciclo de encendido y apagado, según sea
      el turno que corresponda. Al final, vuelve a poner el flag de la depuradora a cero.
   */
   if (depuradoraOn==0){
      // En caso de que la depuradora esté apagada
      PCF8583_write_byte(PCF8583_CTRL_STATUS_REG, 0x04); // Pone el flag de alarma del PCF8583 a 0.
      dtalarm.hours=horaFin;                             // Modifica la hora de la variable dtalarm
      dtalarm.minutes=minutoFin;                         // También los minutos
      PCF8583_set_alarmtime(&dtalarm);                   // Esta nueva hora se introduce al PCF8583 (será la correspondiente a la próxima
                                                         // vez que debe avisar al PIC -> cuando acabe el ciclo de trabajo de la depuradora)
      output_high(PIN_C5);                               // Activa el pin de salida del motor de la depuradora
      depuradoraOn=1;                                    // Ahora el flag indica que la depuradora no está funcionando
   }
   else{
      // En caso de que esté funcionando
      dosificadores();                                   // Se inicia el método que determina si es necesario o no dosificar cloro y floculante
      PCF8583_write_byte(PCF8583_CTRL_STATUS_REG, 0x04); // Pone el flag de alarma del PCF8583 a 0
      dtalarm.hours=horaInicio;                          // Modifica la hora de la variable dtalarm
      dtalarm.minutes=minutoInicio;                      // También los minutos
      PCF8583_set_alarmtime(&dtalarm);                   // Ahora el PCF8583 tendrá como nueva alarma la hora de inicio del próximo ciclo
      output_low(PIN_C5);                                // Se pone a 0 el pin de salida al motor de la depuradora
      depuradoraOn=0;                                    // El flag vuelve a indicar que la depuradora no está funcionando
   }
   flagDepuradora=0;
}

//=================================
void main()
{
   // Ambas interrupciones externas estarán disparadas por flanco de bajada
   ext_int_edge(0,H_TO_L);
   ext_int_edge(1,H_TO_L);
   // Se habilitan las interrupciones
   enable_interrupts(GLOBAL);
   enable_interrupts(INT_EXT);
   enable_interrupts(INT_EXT1);
   enable_interrupts(INT_TIMER1);
   
   // Configuración
   setup_adc_ports(NO_ANALOGS);
   setup_adc(ADC_OFF);
   // Configuración de los timers
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);  // Prescaler de 1 en el timer 0, que será el que afecte al wdt
   setup_timer_1(T1_INTERNAL | T1_DIV_BY_8); // Timer1 con prescaler de 8, y se considera el reloj interno
   setup_timer_2(T2_DISABLED,0,1);           // Timer2 inactivo
   setup_comparator(NC_NC_NC_NC);
   setup_vref(VREF_LOW|-2);
   setup_adc_ports(AN0_TO_AN3);  // RA0 a RA3 entradas analógicas (ver 18F4520.h)
   setup_adc(adc_clock_div_32);  // Ajusta tiempo de conversión de cada bit

   set_timer1(15536);   // Se introducen 15536 de inicio al timer1
   lcd_init();          // Se inicializa el LCD
   lcd_send_byte(0, 1); // Borra LCD
   printf("Bienvenido al sistema de control de la piscina\n\r\n\r\n\r");
   leeEeprom();         // Se cargarán de EEPROM varias variables para el funcionamiento de la piscina. Concretamente los
                        // valores deseados de temperatura, cloro y turbidez, y la hora de encendido y apagado del motor de
                        // la depuradora.

   // Se pedirá que se introduzca la hora inicial al sistema
   lcd_putc("Hora actual?");
   lcd_gotoxy(1,2);
   lcd_putc("Formato: XX:XX");
   lcd_gotoxy(10,2);
   ntpl=0;
   while(ntpl<4){
      // Según las teclas que haya pulsado (iré por un dígito u otro de las horas:minutos)
      switch(ntpl){
         case 0:  n1=kbd_getc();
         if (n1<=2){
            printf(lcd_putc,"%u",n1);
            ntpl++;
            if (n1==2)
               auxiliar=4;
            else
               auxiliar=10;
         }
         break;
         case 1:  n2=kbd_getc();
         if (n2<auxiliar){
            printf(lcd_putc,"%u:",n2);
            ntpl++;
         }
         break;
         case 2:  n3=kbd_getc();
         if (n3<=5){
            printf(lcd_putc,"%u",n3);
            ntpl++;
         }
         break;
         case 3:  n4=kbd_getc();
         if (n4<10){
            printf(lcd_putc,"%u",n4);
            ntpl++;
         }
         break;
      }
   }
   // Inicialización de los flags
   flag=0;           // El flag del menú empezará inactivo (para acceder al menú habrá que pulsar su botón)
   flagDepuradora=0; // Y lo mismo el flag de activación o desactivación del motor de la depuradora
   depuradoraOn=0;   // La depuradora se supone apagada de inicio
   
   // Introducción de la hora elegida
   dt.hours=10*n1+n2;
   dt.minutes=10*n3+n4;
   PCF8583_init();
   PCF8583_set_datetime(&dt);    // Puesta en fecha-hora*/
   PCF8583_write_byte(PCF8583_CTRL_STATUS_REG, 0x04);    // Activamos los campos de alarma
   PCF8583_write_byte(PCF8583_ALARM_CONTROL_REG, 0x90);  // Introducimos alarma diara
   delay_ms(30);

   lcd_send_byte(0,1);
   lcd_send_byte(0,2);


   while(1){
      /*
         Esta será la parte del programa que estará ejecutándose permanentemente
      */
      comprobar_temperatura();   // Comprueba si debe o no encender el calentador del agua

      if (flag!=1 && flagDepuradora!=1){                 // Si no están activos los flags del menú
         mostrar_lcd();             // Muestra datos del estado actual de la piscina por el LCD
         mostrar_rs232();           // Muestra los valores guardados en Eeprom, por la interfaz RS232
      }
      if (flag==1)
         menu();                    // Si el flag está activo, muestra el menú de usuario

      if (flagDepuradora==1){
         funcionaDepuradora();
      }
      if (difCloro<=0.0 && difTurbidez<=0.0)
         sleep();                   // Si no está dosificando cloro o floculante, el PIC duerme, hasta que le
                                    // despierte el perro guardián o alguna interrupción externa

   }


}
