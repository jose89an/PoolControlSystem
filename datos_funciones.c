/*
   En este archivo predefino todos los datos y m�todo que usar� a lo largo del programa. Esto se hace as� por
   la forma en que se ejecutan los archivos en C, que necesitan que los m�todos o las variables hayan sido
   declaradas previamente, aunque no tomen ning�n valor (o m�todo vac�os). Sin embargo, los m�todos tendr�n
   el funcionamiento que se les d� m�s adelante al definirlos de nuevo.
*/

// Niveles de temperatura, cloro y turbidez deseada, adem�s de las variables con las que se controlar� la dosificaci�n de cloro y floculante
float tempDeseada,cloroDeseado,turbidezDeseada,difCloro, difTurbidez;
// Variables de inicio y fin del ciclo de la depuradora, flag del men� e indicador de depuradora activa
int horaInicio, minutoInicio, horaFin, minutoFin,flag,flagDepuradora,depuradoraOn,auxiliar,ntpl,n1,n2,n3,n4;
   // auxiliar es un valor usado para comparar al introducir hora inicial
   // ntpl se usa para comprobar el n�mero de teclas pulsadas al introducir la hora inicial
   // n1-n4 se usan al introducir la hora inicial, para las distintas cifras 
// Aqu� se guardan los d�as de la semana como cadenas de caracteres
char weekday[10];
// Variables del tiempo actual en el PCF y la hora de alarma. Este tipo de dato fue definido en el fichero del PCF8583
date_time_t dt,dtalarm;
// M�todos definidos en feeprom.c
void escribeEeprom();
void leeEeprom();
// M�todos definidos en el archivo principal
void comprobar_temperatura();
void mostrar_lcd();
void mostrar_rs232();
void dosificadores();
void funcionaDepuradora();
// M�todos usados por la interfaz
void menuAlarma();
void menuTemperatura();
void menuCloro();
void menuTurbidez();
void menu();
