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


////////////////////////////////////////////////////////////
//define Constant for power calculation 
////////////////////////////////////////////////////////////


// setup the I2C port
#use i2c(MASTER, I2C1, FORCE_HW)

int1 selectPush =0,selectUsed=1;
int1 exitPush =0,exitUsed =1;
int1 leftPush =0,leftUsed =1;
int1 rightPush =0,rightUsed =1;
//move from main
int menu = 1;
int1 selectmenu = 0;
int16 analog0;
int16 REFERENCE_VALUE = 525;


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
int1 checkselect(int1 menu);
int1 checkexit(int1 menu);
int checkleft(int menu);
int checkright(int menu);
int16 calibrate(int16 analog);
int16 resetcalibrate();
int16 amperecal(int16 analog);
int16 powercal(int16 ampere);
void menu4();
void menu5();

//!///////////////////////////////////////////////////////////////////////////
//!// This is the main device register
//!///////////////////////////////////////////////////////////////////////////
//!int deviceRegister[REGISTER_SIZE]={0}; // device register




void main() {
   
    
 
    float32 power_sim;


    int16 ampere;
    int16 power;
   
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
       
       delay_ms(100); //100
      
       
       
       if(REFERENCE_VALUE>525){
         analog0 = (analog0-(REFERENCE_VALUE-525));
       }
       else if(REFERENCE_VALUE<525){
         analog0 = (analog0+(525-REFERENCE_VALUE));  
       } 
       //decrese or increse analog0 to fit in graph that we solve because we solve at 0,525 at 0 mA
       
    
       ampere = amperecal(analog0);
       power_sim =  0.230*ampere; //((230*ampere)*1000)
       power = (int16) power_sim;
       printf("Sensor value = %Lu\r\n",analog0 );
       printf("AMPERE = %Lu\r\n",ampere );  //sent to computer
       printf("POWER = %Lu\r\n",power );
       //delay_ms(500);
       
   
       
       if(menu>=6){
         
         menu%=6;
         //menu+=1;
       }
       else if(menu<=0)
       { menu=1;
       }
       else if(menu==1){
         menu = checkleft(menu);
         menu = checkright(menu);
         
         setDisplayPos(1);                     
         displayLongText("SENSOR");
         setDisplayPos(7);                     
         displayLongText("  mA  ");
         setDisplayPos(13);                     
         displayLongText("WATT");
         if(analog0<1000){
            setDisplayPos(18);
            displayValue(analog0);
         }
         else{
            setDisplayPos(17);
            displayValue(analog0);
         }
         if(ampere<1000){
            setDisplayPos(21);
            displayLongText("    ");
            setDisplayPos(25);
            displayValue(ampere);
         }
         else{
            setDisplayPos(21);
            displayLongText("   ");
            setDisplayPos(24);
            displayValue(ampere);
         }
         if(power<1000){
            setDisplayPos(28);                     
            displayLongText("  ");
            setDisplayPos(30);
            displayValue(power);
         }
         else{
            setDisplayPos(28);                     
            displayLongText(" ");
            setDisplayPos(29);
            displayValue(power);
         }
         
       
       }
       else if(menu==2){
         menu = checkleft(menu);
         menu = checkright(menu);
         setDisplayPos(1);                     
         displayLongText(" volt ");
         setDisplayPos(7);                     
         displayLongText("  mA  ");
         setDisplayPos(13);                     
         displayLongText("WATT");
         setDisplayPos(18);
         displayValue(230);
         if(ampere<1000){
            setDisplayPos(21);
            displayLongText("    ");
            setDisplayPos(25);
            displayValue(ampere);
         }
         else{
            setDisplayPos(21);
            displayLongText("   ");
            setDisplayPos(24);
            displayValue(ampere);
         }
         if(power<1000){
            setDisplayPos(28);                     
            displayLongText("  ");
            setDisplayPos(30);
            displayValue(power);
         }
         else{
            setDisplayPos(28);                     
            displayLongText(" ");
            setDisplayPos(29);
            displayValue(power);
         }
         
        
       }
       else if(menu ==3){
            menu = checkleft(menu);
            menu = checkright(menu);
       }
       else if(menu ==4){
            menu4();
       }
       else if(menu ==5){
            menu5();
            
       
       }
       
       
  
       
       
         
    }
}


/////////////////
//calculation Function
////////////////
int16 amperecal(int16 analog){
   float32 analog_sim;
   float32 ampere_sim;
   int16 ampere;
   float32 A = 0.000095671;
   float32 B = 0.180762;
   float32 C = 117.297;
   float32 D = 25596.3;
   
   analog_sim = analog;
   ampere_sim =(A*( pow(analog_sim,3)))-(B*( pow(analog_sim,2)))+(C* analog_sim)-D;
   ampere = (int16) ampere_sim;
   return ampere;
}
int16 powercal(int16 ampere){


}




/////////////////
//calculation Function
////////////////



/////////////////
//Menu Function
////////////////


void menu4(){
     selectmenu = checkselect(selectmenu);
            if(selectmenu > 0){
               REFERENCE_VALUE = calibrate(analog0);
               selectmenu = 0;
               setDisplayPos(1);                     
               displayLongText("   CALIBRATE    ");
               setDisplayPos(17);                     
               displayLongText("  OK !!! ");
               delay_ms(200);
            }
            else{
               menu = checkleft(menu);
               menu = checkright(menu);
               setDisplayPos(1);                     
               displayLongText("   CALIBRATE    ");
               setDisplayPos(17);                     
               displayLongText("  This Tool !!! ");
            
            }
}

void menu5(){
   selectmenu = checkselect(selectmenu);
            if(selectmenu > 0){
               REFERENCE_VALUE = resetcalibrate();
               selectmenu = 0;
               setDisplayPos(1);                     
               displayLongText("RESET CALIBRATE");
               setDisplayPos(18);                     
               displayLongText("OK !!!");
               delay_ms(200);
            }
            else{
               menu = checkleft(menu);
               menu = checkright(menu);
               setDisplayPos(1);                     
               displayLongText("RESET CALIBRATE");
               setDisplayPos(18);                     
               displayLongText("This Tool !!!");
            
            }
   

}


/////////////////
//Menu Function
////////////////



/////////////////
//Calibrate Function
////////////////

int16 calibrate(int16 analog){
   return analog; //You could use this function when there's no input electric device.


}
int16 resetcalibrate(){
   return 525;   //return old reference.

}
/////////////////
//Calibrate Function
////////////////


/////////////////
//check button
////////////////
int1 checkselect(int1 menu){
       
      if(selectUsed==0 && selectPush==1){
         selectUsed=1;
         menu+=1;
         clearDisplay();
       }
   
       return menu;
}

int1 checkexit(int1 menu){
       
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

/////////////////
//check button
////////////////








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
