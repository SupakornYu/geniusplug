
#include <16F886.h>
#device adc=10  // use 10-bit ADC
#device PASS_STRINGS=IN_RAM
#include <math.h>
#FUSES NOWDT //No Watch Dog Timer
#FUSES HS //High speed Osc (> 4mhz for PCM/PCH) (>10mhz for PCD)
#FUSES PUT //Power Up Timer
#FUSES NOMCLR //Master Clear pin disabled
#FUSES NOPROTECT //Code not protected from reading
#FUSES NOCPD //No EE protection
#FUSES BROWNOUT //No brownout reset
#FUSES IESO //Internal External Switch Over mode enabled
#FUSES FCMEN //Fail-safe clock monitor enabled
#FUSES NOLVP //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NODEBUG //No Debug mode for ICD
#FUSES NOWRT //Program memory not write protected
#FUSES BORV40 //Brownout reset at 4.0V
#FUSES RESERVED //Used to set the reserved FUSE bits
#use delay(clock=20000000)
// Config the serial port hardware on the PIC
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8)

////////////////////////////////////////////////////////////
//define LCD Display 
////////////////////////////////////////////////////////////
#define SLAVE_ADDRESS  0xB0
#define DISPLAY_ADDRESS 0xB4
#define REGISTER_SIZE  8   
#define DISPLAY_CMD_SEND_VALUE 2
#define DISPLAY_CMD_SEND_LONG_TEXT 5
#define DISPLAY_CMD_CLS 6
#define DISPLAY_CMD_SETPOS 8
////////////////////////////////////////////////////////////
//define LCD Display 
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
//define Constant for power calculation 
////////////////////////////////////////////////////////////

#define SENSITIVE_VOLTAGE 5.625 //voltage per amp
////////////////////////////////////////////////////////////
//define Constant for power calculation 
////////////////////////////////////////////////////////////


// setup the I2C port
#use i2c(MASTER, I2C1, FORCE_HW)

int1 selectPush =0,selectUsed=1;
int1 exitPush =0,exitUsed =1;
int1 leftPush =0,leftUsed =1;
int1 rightPush =0,rightUsed =1;



#INT_RB
void rb_isr(void) {
   if(input(PIN_B3)==0 && selectUsed==0) // input(PIN_B3) == 0 is touching
   {
         selectPush = 1;
   }
   else if(input(PIN_B3)==1 && selectUsed==1)
   {      
         selectPush = 0;
         selectUsed = 0;
   }
   
   else if(input(PIN_B2)==0 && exitUsed==0) // input(PIN_B3) == 0 is touching
   {
         exitPush = 1;
   }
   else if(input(PIN_B2)==1 && exitUsed==1)
   {      
         exitPush = 0;
         exitUsed = 0;
   }
   else if(input(PIN_B1)==0 && leftUsed==0) // input(PIN_B3) == 0 is touching
   {
         leftPush = 1;
   }
   else if(input(PIN_B1)==1 && leftUsed==1)
   {      
         leftPush = 0;
         leftUsed = 0;
   }
   else if(input(PIN_B4)==0 && rightUsed==0) // input(PIN_B3) == 0 is touching
   {
         rightPush = 1;
   }
   else if(input(PIN_B4)==1 && rightUsed==1)
   {      
         rightPush = 0;
         rightUsed = 0;
   }
   //if you wanna check the other button you have to write in this function with input(parameter);
}


void displayLongText(char* text);
void displayValue(int16 value[]);
void clearDisplay();
void setDisplayPos(int pos);
int checkselect(int menu);
int checkexit(int menu);
int checkleft(int menu);
int checkright(int menu);

//!///////////////////////////////////////////////////////////////////////////
//!// This is the main device register
//!///////////////////////////////////////////////////////////////////////////
//!int deviceRegister[REGISTER_SIZE]={0}; // device register




void main() {
   
    int16 analog0;
    int16 analogget0;
    float32 analog0_sim;
    float32 ampere_sim;
    float32 power_sim;
    int16 ampere;
    int16 power;
   
    int16 REFERENCE_VALUE = 525;
    float32 A = 0.000095671;
    float32 B = 0.180762;
    float32 C = 117.297;
    float32 D = 25596.3;
    int menu = 1;
    enable_interrupts(INT_RB3);    // generate interrupt when B7 changes
    enable_interrupts(INT_RB2);
    enable_interrupts(INT_RB1);
    enable_interrupts(INT_RB4);
    enable_interrupts(GLOBAL);
    
    setup_adc_ports(sAN1); // setup PIN A0 as analog input
    setup_adc( ADC_CLOCK_INTERNAL ); 
 
    printf("Sampling:\r\n");
    set_adc_channel( 1 ); // set the ADC channel to read 
    delay_us(100); // wait for the sensor reading to finish
 
    while (1) {
       analog0 = read_adc();
       
       
       
      /* if(analog0 > 500)
         {  
            printf("Light OFF\n");
            output_low(PIN_B6);
         }
       else 
         {  printf("Light ON\n");
            output_high(PIN_B6);
         }*/
       delay_ms(100); //100
      
       
       //test button
       /*
          if(selectUsed==0 && selectPush==1){
         selectUsed=1;
         menu=1;
       }
       else if(exitUsed==0 && exitPush==1){
         exitUsed=1;
         menu=1;
       }
       else if(leftUsed==0 && leftPush==1){
         leftUsed=1;
         menu-=1;
       }
       else if(rightUsed==0 && rightPush==1){
         rightUsed=1;
         menu+=1;
       }
       */
       //setDisplayPos(14);
       //displayValue(i); 
       
       
       
       if(REFERENCE_VALUE>525){
         analog0 = (analog0-(REFERENCE_VALUE-525));
       }
       else if(REFERENCE_VALUE<525){
         analog0 = (analog0+(525-REFERENCE_VALUE));  
       } 
       //decrese or increse analog0 to fit in graph that we solve because we solve at 0,525 at 0 mA
       
      // ampere =  (int16)((analog0 - REFERENCE_VALUE)*SENSITIVE_VOLTAGE); //2.08328889
       
       
       analog0_sim = analog0;
       ampere_sim =(A*( pow(analog0_sim,3)))-(B*( pow(analog0_sim,2)))+(C* analog0_sim)-D;
       ampere = (int16) ampere_sim;
       power_sim =  0.230*ampere; //((230*ampere)*1000)
       power = (int16) power_sim;
       printf("Sensor value = %Lu\r\n",analog0 );
       printf("AMPERE = %Lu\r\n",ampere );  //sent to computer
       printf("POWER = %Lu\r\n",power );
       delay_ms(500);
       //button
   
       menu = checkleft(menu);
       menu = checkright(menu);
       if(menu>6){
         menu%=6;
       }
       else if(menu==1){
         setDisplayPos(1);                     
         displayLongText("SENSOR");
         setDisplayPos(7);                     
         displayLongText("  mA  ");
         setDisplayPos(13);                     
         displayLongText("WATT");
         setDisplayPos(18);
         displayValue(analog0);
         setDisplayPos(21);                     
         displayLongText("   ");
         setDisplayPos(24);
         displayValue(ampere);
         setDisplayPos(28);                     
         displayLongText(" ");
         setDisplayPos(29);
         displayValue(power);
       
       }
       else if(menu==2){
         setDisplayPos(1);                     
         displayLongText(" volt ");
         setDisplayPos(7);                     
         displayLongText("  mA  ");
         setDisplayPos(13);                     
         displayLongText("WATT");
         setDisplayPos(18);
         displayValue(230);
         setDisplayPos(21);                     
         displayLongText("   ");
         setDisplayPos(24);
         displayValue(ampere);
         setDisplayPos(28);                     
         displayLongText(" ");
         setDisplayPos(29);
         displayValue(power);
         
       }
       
       
  
       
       
       
       //clearDisplay();  
    }
}






/////////////////
//check button
////////////////
int checkselect(int menu){
       
      if(selectUsed==0 && selectPush==1){
         selectUsed=1;
         menu=1;
         clearDisplay();
       }
       else{
         menu=0;
       
       }
       return menu;
}

int checkexit(int menu){
       
       if(exitUsed==0 && exitPush==1){
         exitUsed=1;
         menu=1;
         clearDisplay();
       }
       else{
         menu=0;
       
       }
       return menu;
}

int checkleft(int menu){
       
       if(leftUsed==0 && leftPush==1){
         leftUsed=1;
         menu-=1;
         clearDisplay();
       }
       
       return menu;
}

int checkright(int menu){
       
       if(rightUsed==0 && rightPush==1){
         rightUsed=1;
         menu+=1;
         clearDisplay();
       }
       
       return menu;
}










///////////////////////////////////////////////////////////////////////////
//LCD Display Funtion 
///////////////////////////////////////////////////////////////////////////
void displayLongText(char* text) {

int i ;
i2c_start();
i2c_write(DISPLAY_ADDRESS);
i2c_write(DISPLAY_CMD_SEND_LONG_TEXT);

for(i=0;text[i]!='\0';i++)
{
i2c_write(text[i]);

}
i2c_write('\0');
i2c_stop();

//delay_ms(100);

}


void displayValue(int16 value[]) {

   i2c_start();
   i2c_write(DISPLAY_ADDRESS);
   i2c_write(DISPLAY_CMD_SEND_VALUE);
   i2c_write((int)(value>>8)); // high byte
   i2c_write((int)(value & 0xFF)); // low byte
   i2c_write('\0');
   i2c_stop();
   
}

void clearDisplay(){

   i2c_start();
   i2c_write(DISPLAY_ADDRESS);
   i2c_write(DISPLAY_CMD_CLS);
   i2c_stop();
}

void setDisplayPos(int pos){
   i2c_start();
   i2c_write(DISPLAY_ADDRESS);
   i2c_write(DISPLAY_CMD_SETPOS);
   i2c_write(pos);
   i2c_write('\0');
   i2c_stop();
}
///////////////////////////////////////////////////////////////////////////
//LCD Display Funtion 
///////////////////////////////////////////////////////////////////////////