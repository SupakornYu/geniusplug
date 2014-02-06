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

//PIC16 Write Internal EEPROM
#rom 0x2100={0x00}

int16 readUnitValueInEEPROM();
void writeUnitValueInEEPROM();
void clearUnitValueInEEPROM();

int1 selectPush =0,selectUsed=1;
int1 exitPush =0,exitUsed =1;
int1 leftPush =0,leftUsed =1;
int1 rightPush =0,rightUsed =1;
//move from main
int menu = 1;
int1 selectmenu = 0;
int16 analog0;
int16 REFERENCE_VALUE = 525;
int16 power = 0;
int checkfor_unit = 0;
int16 unit = readUnitValueInEEPROM();
int1 billcheck = 0;
float32 unit_sim = (float32) unit; //write function get from eerom here
float32 bill_sim = 0;
int16 bill = 0;
int1 status_power_on = 0;
int16 power_off_time = 0;
int16 power_off_timeinterrupt = 0;
int1 power_off_timeUsed = 0;

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


#INT_TIMER1
void timer1_isr()
{
   set_timer1(3036); // 
   checkfor_unit+=1;
   if(checkfor_unit > 49)
   {
     checkfor_unit =0;
     billcheck = 1;
     
   }
   
   if(power_off_timeUsed == 1){
      power_off_timeinterrupt+=1;
      if(power_off_timeinterrupt >(power_off_time*10)){
         status_power_on = 0;
         power_off_timeinterrupt = 0;
         power_off_time = 0;
         power_off_timeUsed = 0;
      }
   }
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
void powercal(int16 ampere);
void unitcal();
void menucalibrate();
void menuresetcalibrate();
void billcal();
void menushutdown();
void menushutdownfortime10second();

void menuresetunit();

//!///////////////////////////////////////////////////////////////////////////
//!// This is the main device register
//!///////////////////////////////////////////////////////////////////////////
//!int deviceRegister[REGISTER_SIZE]={0}; // device register




void main() {
   
    
 
    


    int16 ampere;
    
    setup_timer_1(T1_INTERNAL | T1_DIV_BY_8);
    set_timer1(3036); 
    enable_interrupts(INT_TIMER1);
    
   
    enable_interrupts(INT_RB3);    // generate interrupt when B7 changes
    enable_interrupts(INT_RB2);
    enable_interrupts(INT_RB1);
    enable_interrupts(INT_RB4);
    enable_interrupts(GLOBAL);
    
    setup_adc_ports(sAN1); // setup PIN A0 as analog input
    setup_adc( ADC_CLOCK_INTERNAL ); 
 
    output_low(PIN_C5); //off Relay
    output_low(PIN_B5); //off led for power off status
    //printf("Sampling:\r\n");
    set_adc_channel( 1 ); // set the ADC channel to read 
    delay_us(100); // wait for the sensor reading to finish
 
    while (1) {
       
     if(status_power_on<= 0){
       menu=1;
       
       output_low(PIN_B0);
       delay_ms(200);
       output_low(PIN_B6);
       status_power_on = checkselect(status_power_on);
       setDisplayPos(5);                     
       displayLongText("Welcome"); 
       delay_ms(100);
       if(billcheck == 1){
         
         billcheck = 0;
         clearDisplay(); //if you don't want to clear display every time follow sampling unit.You can erase here.
       
       }
     
     }
     else{
       output_high(PIN_B6);
       output_high(PIN_B0);
       delay_ms(100);
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
       powercal(ampere);
       if(billcheck == 1){
         unitcal();
         billcheck = 0;
         clearDisplay(); //if you don't want to clear display every time follow sampling unit.You can erase here.
       
       }
       billcal();
       //printf("Sensor value = %Lu\r\n",analog0 );
       //printf("AMPERE = %Lu\r\n",ampere );  //sent to computer
       //printf("POWER = %Lu\r\n",power );
       //delay_ms(500);
       
   
       
       if(menu>=9){
         
         menu%=9;
         //menu+=1;
       }
       else if(menu<=0)
       { menu=1;
       }
       else if(menu==1){
         
         
         setDisplayPos(1);                     
         displayLongText("SENSOR");
         setDisplayPos(9);                     
         displayLongText("mA");
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
         menu = checkleft(menu);
         menu = checkright(menu);
         
         
       
       }
       else if(menu==2){
         
         setDisplayPos(2);                     
         displayLongText("volt");
         setDisplayPos(9);                     
         displayLongText("mA");
         setDisplayPos(13);                     
         displayLongText("WATT");
         setDisplayPos(18);
         displayValue(230);
         
         setDisplayPos(21);
         displayLongText("   ");
         setDisplayPos(24);
         displayValue(ampere);
         delay_us(100);
         setDisplayPos(28);                     
         displayLongText(" ");
         setDisplayPos(29);
         displayValue(power);
         delay_us(100);
         menu = checkleft(menu);
         menu = checkright(menu);
         
        
       }
       else if(menu ==3){
            
            setDisplayPos(1);
            displayLongText(" Unit");
            setDisplayPos(8);
            displayValue(unit);
            setDisplayPos(17);
            displayLongText(" Baht");
            setDisplayPos(24);
            displayValue(bill);
            menu = checkleft(menu);
            menu = checkright(menu);
       }
       else if(menu ==4){
            menucalibrate();
       }
       else if(menu ==5){
            menuresetcalibrate();
       }
       else if(menu ==6){
            menuresetunit();
       }
       else if(menu ==7){
            menushutdown();
       }
       else if(menu ==8){
            menushutdownfortime10second();
       }
       
       
       
  
       
       
     }   
    }
}


/////////////////////////////
//Unit Value memory Function
/////////////////////////////
int16 readUnitValueInEEPROM(){
   
   int16 readUnit;
   
   readUnit = read_eeprom(0x00) << 8; // address 10 is high_byte
   readUnit += read_eeprom(0x01) ;    // address 11 is low_byte
   return readUnit;
}


void writeUnitValueInEEPROM(){
   int con_h,con_l;
   
   con_l = unit & 0xff;
   con_h = unit >> 8;
   
   write_eeprom(0x00,con_h);
   write_eeprom(0x01,con_l);

}

void clearUnitValueInEEPROM(){
   unit = 0;
   unit_sim = (float32) unit;
   write_eeprom(0x00,0);
   write_eeprom(0x01,0);

}



/////////////////////////////
//Unit Value memory Function
/////////////////////////////








/////////////////
//calculation Function
////////////////
void powercal(int16 ampere){
   float32 power_sim = 0;
   power_sim =  0.230*ampere; //((230*ampere)*1000)
   power = (int16) power_sim;
}


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

void unitcal(){
   //unit+=1;
   //unit_sim = unit_sim + (power*1.388E-6); //(power*0.001*(5/3600))
   unit_sim = unit_sim + (power*1.388E1);
   unit = (int16) unit_sim;
   writeUnitValueInEEPROM();
}

void billcal(){
   bill_sim = unit_sim * 3;
   bill = (int16) bill_sim;
}

/////////////////
//calculation Function
////////////////



/////////////////
//Menu Function
////////////////
void menushutdownfortime10second(){
     
     selectmenu = checkselect(selectmenu);  
            if(selectmenu > 0){
               //save eeprom here
               setDisplayPos(1);                     
               displayLongText("OK");
               power_off_time = 10;
               power_off_timeUsed = 1;
               selectmenu = 0;
            }
            else{
              
               setDisplayPos(1);                     
               displayLongText("OFF 10 s");
               setDisplayPos(17);
               displayValue(power_off_timeinterrupt*0.1);
               menu = checkleft(menu);
               menu = checkright(menu);
            }
     
}





void menushutdown(){
      selectmenu = checkexit(selectmenu);  
            if(selectmenu > 0){
               //save eeprom here
               status_power_on = 0;
               selectmenu = 0;
            }
            else{
              
               setDisplayPos(1);                     
               displayLongText("POWER OFF");
               menu = checkleft(menu);
               menu = checkright(menu);
            }


}





void menuresetunit(){
      selectmenu = checkselect(selectmenu);
            if(selectmenu > 0){
               clearUnitValueInEEPROM();
               selectmenu = 0;
               setDisplayPos(1);                     
               displayLongText("Clear OK");
               delay_ms(200);
            }
            else{
               
               setDisplayPos(1);                     
               displayLongText("RESET Unit&Money");
               menu = checkleft(menu);
               menu = checkright(menu);
            }


}

void menucalibrate(){
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
               
               setDisplayPos(1);                     
               displayLongText("   CALIBRATE    ");
               setDisplayPos(17);                     
               displayLongText("  This Tool !!! ");
               menu = checkleft(menu);
               menu = checkright(menu);
            
            }
}

void menuresetcalibrate(){
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
               
               setDisplayPos(1);                     
               displayLongText("RESET CALIBRATE");
               setDisplayPos(18);                     
               displayLongText("This Tool !!!");
               menu = checkleft(menu);
               menu = checkright(menu);
            
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
         delay_us(100);
       }
   
       return menu;
}

int1 checkexit(int1 menu){
       
       if(exitUsed==0 && exitPush==1){
         exitUsed=1;
         menu+=1;
         clearDisplay();
         delay_us(100);
       }
       else{
         return menu;
       
       }
       return menu;
}

int checkleft(int menu){
       
       if(leftUsed==0 && leftPush==1){
         leftUsed=1;
         menu-=1;
         clearDisplay();
         delay_us(100);
       }
       
       return menu;
}

int checkright(int menu){
       
       if(rightUsed==0 && rightPush==1){
         rightUsed=1;
         menu+=1;
         clearDisplay();
         delay_us(100);
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
