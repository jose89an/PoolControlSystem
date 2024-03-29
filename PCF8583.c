// Funciones para el RTC I2c: PCF8583
// Este c�digo no procede de CCS (no est� totalmente verificado)
// Inluye funciones de conversi�n BCD-Binario
// y de lectura/escritura a trav�s de I2C.

//definicion de direcciones
#ifndef PCF8583_SDA
#define PCF8583_SDA  PIN_C4
#define PCF8583_SCL  PIN_C3
#endif

#use i2c(master, sda=PCF8583_SDA, scl=PCF8583_SCL)

#ifndef PCF8583_WRITE_ADDRESS
#define PCF8583_WRITE_ADDRESS 0xA0
#define PCF8583_READ_ADDRESS  0xA1
#endif

// Register addresses
#define PCF8583_CTRL_STATUS_REG    0x00
#define PCF8583_100S_REG           0x01
#define PCF8583_SECONDS_REG        0x02
#define PCF8583_MINUTES_REG        0x03
#define PCF8583_HOURS_REG          0x04
#define PCF8583_DATE_REG           0x05
#define PCF8583_MONTHS_REG         0x06
#define PCF8583_TIMER_REG          0x07

#define PCF8583_ALARM_CONTROL_REG  0x08
#define PCF8583_ALARM_100S_REG     0x09
#define PCF8583_ALARM_SECS_REG     0x0A
#define PCF8583_ALARM_MINS_REG     0x0B
#define PCF8583_ALARM_HOURS_REG    0x0C
#define PCF8583_ALARM_DATE_REG     0x0D
#define PCF8583_ALARM_MONTHS_REG   0x0E
#define PCF8583_ALARM_TIMER_REG    0x0F

// Use the first NVRAM address for the year byte.
#define PCF8583_YEAR_REG           0x10


// Commands for the Control/Status register.
#define PCF8583_START_COUNTING     0x00
#define PCF8583_STOP_COUNTING      0x80
#define PCF8583_ENABLEINTERRUPTS   0x02
#define PCF8583_ENABLEALARM        0x91



char const weekday_names[7][10] =
{
{"DOM"},
{"LUN"},
{"MAR"},
{"MIE"},
{"JUE"},
{"VIE"},
{"SAB"}
};

// This structure defines the user's date and time data.
// The values are stored as unsigned integers.  The user
// should declare a structure of this type in the application
// program. Then the address of the structure should be
// passed to the PCF8583 read/write functions in this
// driver, whenever you want to talk to the chip.
typedef struct
{
int8 seconds;    // 0 to 59
int8 minutes;    // 0 to 59
int8 hours;      // 0 to 23  (24-hour time)
int8 day;        // 1 to 31
int8 month;      // 1 to 12
int8 year;       // 00 to 99
int8 weekday;    // 0 = Sunday, 1 = Monday, etc.
}date_time_t;


//----------------------------------------------
void PCF8583_write_byte(int8 address, int8 data)
{
//disable_interrupts(GLOBAL);  //antes de escribir deshabilito interrupciones, y al final las habilito de nuevo para evitar conflictos
i2c_start();
i2c_write(PCF8583_WRITE_ADDRESS);
i2c_write(address);
i2c_write(data);
i2c_stop();
//enable_interrupts(GLOBAL);
}

//----------------------------------------------
int8 PCF8583_read_byte(int8 address)
{
int8 retval;

//disable_interrupts(GLOBAL);
i2c_start();
i2c_write(PCF8583_WRITE_ADDRESS);
i2c_write(address);
i2c_start();
i2c_write(PCF8583_READ_ADDRESS);
retval = i2c_read(0);
i2c_stop();
//enable_interrupts(GLOBAL);
return(retval);
}


void PCF8583_init(void)
{
PCF8583_write_byte(PCF8583_CTRL_STATUS_REG,
                              PCF8583_START_COUNTING);
}

void PCF8583_init_enableinterrupts(void)
{
PCF8583_write_byte(PCF8583_CTRL_STATUS_REG,
                              PCF8583_ENABLEINTERRUPTS);
}

//----------------------------------------------
// This function converts an 8 bit binary value
// to an 8 bit BCD value.
// The input range must be from 0 to 99.

int8 bin2bcd(int8 value)
{
char retval;

retval = 0;

while(1)
  {
   // Get the tens digit by doing multiple subtraction
   // of 10 from the binary value.
   if(value >= 10)
     {
      value -= 10;
      retval += 0x10;
     }
   else // Get the ones digit by adding the remainder.
     {
      retval += value;
      break;
     }
   }

return(retval);
}

//----------------------------------------------
// This function converts an 8 bit BCD value to
// an 8 bit binary value.
// The input range must be from 00 to 99.

char bcd2bin(char bcd_value)
{
char temp;

temp = bcd_value;

// Shifting the upper digit right by 1 is
// the same as multiplying it by 8.
temp >>= 1;

// Isolate the bits for the upper digit.
temp &= 0x78;

// Now return: (Tens * 8) + (Tens * 2) + Ones
return(temp + (temp >> 2) + (bcd_value & 0x0f));

}

//----------------------------------------------
void PCF8583_set_datetime(date_time_t *dt)
{
int8 bcd_sec;
int8 bcd_min;
int8 bcd_hrs;
int8 bcd_day;
int8 bcd_mon;

// Convert the input date/time into BCD values
// that are formatted for the PCF8583 registers.
bcd_sec = bin2bcd(dt->seconds);
bcd_min = bin2bcd(dt->minutes);
bcd_hrs = bin2bcd(dt->hours);
bcd_day = bin2bcd(dt->day) | (dt->year << 6);
bcd_mon = bin2bcd(dt->month) | (dt->weekday << 5);

// Stop the RTC from counting, before we write to
// the date and time registers.
PCF8583_write_byte(PCF8583_CTRL_STATUS_REG,
                              PCF8583_STOP_COUNTING);

// Write to the date and time registers.  Disable interrupts
// so they can't disrupt the i2c operations.
//disable_interrupts(GLOBAL);
i2c_start();
i2c_write(PCF8583_WRITE_ADDRESS);
i2c_write(PCF8583_100S_REG);   // Start at 100's reg.
i2c_write(0x00);               // Set 100's reg = 0
i2c_write(bcd_sec);
i2c_write(bcd_min);
i2c_write(bcd_hrs);
i2c_write(bcd_day);
i2c_write(bcd_mon);
i2c_stop();
//enable_interrupts(GLOBAL);

// Write the year byte to the first NVRAM location.
// Leave it in binary format.
PCF8583_write_byte(PCF8583_YEAR_REG, dt->year);

// Now allow the PCF8583 to start counting again.
PCF8583_write_byte(PCF8583_CTRL_STATUS_REG,
                              PCF8583_START_COUNTING);
}


void PCF8583_set_alarmtime(date_time_t *dt)
{
int8 bcd_sec;
int8 bcd_min;
int8 bcd_hrs;
int8 bcd_day;
int8 bcd_mon;

// Convert the input date/time into BCD values
// that are formatted for the PCF8583 registers.
bcd_sec = bin2bcd(dt->seconds);
bcd_min = bin2bcd(dt->minutes);
bcd_hrs = bin2bcd(dt->hours);
bcd_day = bin2bcd(dt->day) | (dt->year << 6);
bcd_mon = bin2bcd(dt->month) | (dt->weekday << 5);

//disable_interrupts(GLOBAL);

PCF8583_write_byte(PCF8583_ALARM_100S_REG,0x00);
PCF8583_write_byte(PCF8583_ALARM_SECS_REG,bcd_sec);
PCF8583_write_byte(PCF8583_ALARM_MINS_REG,bcd_min);
PCF8583_write_byte(PCF8583_ALARM_HOURS_REG,bcd_hrs);
PCF8583_write_byte(PCF8583_ALARM_DATE_REG,bcd_day);
PCF8583_write_byte(PCF8583_ALARM_MONTHS_REG,bcd_mon);

//enable_interrupts(GLOBAL);
}



//----------------------------------------------
// Read the Date and Time from the hardware registers
// in the PCF8583.   We don't have to disable counting
// during read operations, because according to the data
// sheet, if any of the lower registers (1 to 7) is read,
// all of them are loaded into "capture" registers.
// All further reading within that cycle is done from
// those registers.

void PCF8583_read_datetime(date_time_t *dt)
{
int8 year_bits;
int8 year;

int8 bcd_sec;
int8 bcd_min;
int8 bcd_hrs;
int8 bcd_day;
int8 bcd_mon;

// Disable interrupts so the i2c process is not disrupted.
//disable_interrupts(GLOBAL);

// Read the date/time registers inside the PCF8583.
i2c_start();
i2c_write(PCF8583_WRITE_ADDRESS);
i2c_write(PCF8583_SECONDS_REG);   // Start at seconds reg.
i2c_start();
i2c_write(PCF8583_READ_ADDRESS);

bcd_sec = i2c_read();
bcd_min = i2c_read();
bcd_hrs = i2c_read();
bcd_day = i2c_read();
bcd_mon = i2c_read(0);
i2c_stop();

//enable_interrupts(GLOBAL);

// Convert the date/time values from BCD to
// unsigned 8-bit integers.  Unpack the bits
// in the PCF8583 registers where required.
dt->seconds = bcd2bin(bcd_sec);
dt->minutes = bcd2bin(bcd_min);
dt->hours   = bcd2bin(bcd_hrs & 0x3F);
dt->day     = bcd2bin(bcd_day & 0x3F);
dt->month   = bcd2bin(bcd_mon & 0x1F);
dt->weekday = bcd_mon >> 5;
year_bits   = bcd_day >> 6;

// Read the year byte from NVRAM.
// This is an added feature of this driver.
year = PCF8583_read_byte(PCF8583_YEAR_REG);

// Check if the two "year bits" were incremented by
// the PCF8583.  If so, increment the 8-bit year
// byte (read from NVRAM) by the same amount.
while(year_bits != (year & 3))
      year++;

dt->year = year;

// Now update the year byte in the NVRAM
// inside the PCF8583.
PCF8583_write_byte(PCF8583_YEAR_REG, year);

}
