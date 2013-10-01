/*
   En este archivo predefino todos los datos y método que usaré a lo largo del programa. Esto se hace así por
   la forma en que se ejecutan los archivos en C, que necesitan que los métodos o las variables hayan sido
   declaradas previamente, aunque no tomen ningún valor (o método vacíos). Sin embargo, los métodos tendrán
   el funcionamiento que se les dé más adelante al definirlos de nuevo.
*/

// Niveles de temperatura, cloro y turbidez deseada, además de las variables con las que se controlará la dosificación de cloro y floculante
float tempDeseada,cloroDeseado,turbidezDeseada,difCloro, difTurbidez;
// Variables de inicio y fin del ciclo de la depuradora, flag del menú e indicador de depuradora activa
int horaInicio, minutoInicio, horaFin, minutoFin,flag,flagDepuradora,depuradoraOn,auxiliar,ntpl,n1,n2,n3,n4;
   // auxiliar es un valor usado para comparar al introducir hora inicial
   // ntpl se usa para comprobar el número de teclas pulsadas al introducir la hora inicial
   // n1-n4 se usan al introducir la hora inicial, para las distintas cifras 
// Aquí se guardan los días de la semana como cadenas de caracteres
char weekday[10];
// Variables del tiempo actual en el PCF y la hora de alarma. Este tipo de dato fue definido en el fichero del PCF8583
date_time_t dt,dtalarm;
// Métodos definidos en feeprom.c
void escribeEeprom();
void leeEeprom();
// Métodos definidos en el archivo principal
void comprobar_temperatura();
void mostrar_lcd();
void mostrar_rs232();
void dosificadores();
void funcionaDepuradora();
// Métodos usados por la interfaz
void menuAlarma();
void menuTemperatura();
void menuCloro();
void menuTurbidez();
void menu();
