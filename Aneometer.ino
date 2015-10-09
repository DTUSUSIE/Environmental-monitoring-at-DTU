/* Sketch to read David Wind Vane  
   de ON7EQ Dec 2011

    To disable interrupts:
    cli();                // disable global interrupts 
    and to enable them:  
    sei();                // enable interrupts


=================================================================== 
DAVIS Vantage Pro & Vantage Pro 2    Wind Sensor (speed & direction
===================================================================

On RJ-45 plug terminals:

Black =  pulse from anemometer. Connect to Digital 2 pin, and use a 4k7 resistor as pull up to + 5v.
         use a 10 to 22nF capacitor from pin D2 to ground to debounce the reed switch of anemometer
       
Red =    Ground

Green =  Wiper  of Wind direction vane - connect to A0.  Use a 1 ... 10 µF / 16v capacitor between A0 and ground (observe C polarity) to avoid jitter

Yellow = + 5v (reference of potentiometer)

*/

// include EEPROM write - required to memorize antenna / band config.
#include <EEPROM.h>

#include <math.h>


int Direction ; // Wind direction

#define PotPin (A0)    // define the input pin for the wind vane potentiometer
int PotValue = 0;      // variable to store the value coming from the potentiometer
int DirCorr = 0;       // Correction on direction ( - 360  to + 360)

#define CalPin (A1)    // define the input pin to initiate direction calibration @ startup. Ground pin to calibrate
byte DirCorrB1 = 0;    // 2 bytes of DirCorr
byte DirCorrB2 = 0;

volatile unsigned long RPMTops;  // RPM tops counter in interrupt routine                             
volatile unsigned long ContactTime;  // Timer to avoid conatct bounce in interrupt routine                              

float RPM;       // RPM count
int metersPerSecond;
float TEMP;      // Temp
                                    
#define RPMsensor (2)      //The pin location of the anemometer sensor

float temp = (0);


////////////////////////////////////////////////////////////////////
void setup() { 

//  Clean EEPROM
//  EEPROM.write (1, 0);
//  EEPROM.write (2, 0);
   
  Serial.begin(57600);
  
  delay (100);
  
// CALIBRATE if button depressed at startup !

//  if ((analogRead(CalPin)<512)) calibrate ();


// else retrieve CAL vales from EEPROM

  DirCorrB1 = EEPROM.read(1);
  DirCorrB2 = EEPROM.read(2);  
  
  DirCorr = (DirCorrB1) + (DirCorrB2);

  

pinMode(RPMsensor, INPUT); 
attachInterrupt(0, rpm, FALLING); 

Serial.print("Aneometer initialized\n\n");

    
 } 

/////////////////////////////////////////////////////////////////////////////////

void loop() { 



// Wind Direction
  
  PotValue = analogRead(PotPin);     // read the value from the potmeter
  Direction = map(PotValue, 0, 1023, 0, 359); 
  Direction = Direction + DirCorr + 3;   // Correct for offset & 5° precision

 convert:       // Convert to 360°  
  
  if (Direction < 0) {
    Direction = Direction + 360;
    goto convert;
  }
  
  if (Direction > 360) {
    Direction = Direction - 360;
    goto convert;
  }
    
  if (Direction == 360) Direction = 0;
  
      Serial.print("Direction:\t");
      //if (Direction < 100)   Serial.print(" ");
      //if (Direction < 10)    Serial.print(" ");            
      Serial.print(((Direction/5)*5), DEC);    // 5° precision is enough to print te direction value
      Serial.print(" degrees ");
 
 
   if ((Direction)<23) {
    Serial.print(" N");
    } 
   if ((Direction>22) && (Direction<68)) {
    Serial.print("NE");
    } 
   if ((Direction>67) && (Direction<113)) {
    Serial.print(" E");
    } 
   if ((Direction>112) && (Direction<158)) {
    Serial.print("SE");
    } 
   if ((Direction>157) && (Direction<203)) {
    Serial.print(" S");
    } 
    if ((Direction>202) && (Direction<247)) {
    Serial.print("SW");
    } 
    if ((Direction>246) && (Direction<292)) {
    Serial.print(" W");
    } 
    if ((Direction>291) && (Direction<337)) {
    Serial.print("NW");
    } 
    if ((Direction>336) && (Direction<=360)) {
    Serial.print(" N");
    } 

    Serial.print("\n\n");
      
  
  // measure RPM
  
   RPMTops = 0;   //Set NbTops to 0 ready for calculations
   sei();         //Enables interrupts
   delay (3000);  //Wait 3 seconds to average
   cli();         //Disable interrupts

 
 // convert to km/h
 

 if ((RPMTops >= 0) and (RPMTops <= 21)) RPM = RPMTops * 1.2;
 if ((RPMTops > 21) and (RPMTops <= 45)) RPM = RPMTops * 1.15;
 if ((RPMTops > 45) and (RPMTops <= 90)) RPM = RPMTops * 1.1;
 if ((RPMTops > 90) and (RPMTops <= 156)) RPM = RPMTops * 1.0;
 if ((RPMTops > 156) and (RPMTops <= 999)) RPM = RPMTops * 1.0;

     
 // print the speed 
 metersPerSecond = (RPM*1000)/3600;

 Serial.print("Speed:\t");
Serial.print(int(metersPerSecond), DEC);
Serial.print("m/s\n");

Serial.print("Speed:\t");
//  if (RPM < 100)   Serial.print(" ");
//  if (RPM < 10)    Serial.print(" ");            
  Serial.print(int(RPM), DEC);
  Serial.print("km/t\n");
  

}


//// This is the function that interrupt calls to measure  RPM  

 void rpm ()   { 

    if ((millis() - ContactTime) > 15 ) {  // debounce of REED contact. With 15ms speed more than 150 km/h can be measured
      RPMTops++; 
      ContactTime = millis();
    }
    
} 
//// end of RPM measure  
 
  
//// This is the function that calibrates the vane

// void calibrate () {
//  
//  Serial.print("Hold to calibr !"); 
//  delay (2000);  //Wait 2 second
//  if ((analogRead(CalPin)>512)) setup();  // CAL not really required ... abort !
//
//  lcd.setCursor(0, 1);  
//  lcd.print("Now calibrating ...    "); 
//  delay (1000);  //Wait 1 second
//  
//  PotValue = analogRead(PotPin);     // read the value from the potmeter
//  DirCorr = map(PotValue, 0, 1023, 359, 0);
//
//  lcd.setCursor(0, 1);  
//  lcd.print("CAL value = "); 
//  lcd.print(DirCorr, DEC); 
//  lcd.print("            ");  
//  delay (2000);  //Wait 2 seconds  
//  
////  
//  DirCorrB1 = DirCorr / 255;
//  if (DirCorrB1 == 1){ 
//  DirCorrB1 = 255;  
//  DirCorrB2 = DirCorr - 255 ;
//    }
//  else {
//  DirCorrB1 = DirCorr;  
//  DirCorrB2 = 0;
//    }   
////
////  DirCorrB1 = DirCorr;  
////  DirCorrB2 = 0;
//  EEPROM.write (1, DirCorrB1);
//  EEPROM.write (2, DirCorrB2);
//
// wait:
//  lcd.setCursor(0, 1);  
//  lcd.print("CAL OK - Release !   ");  
//  if ((analogRead(CalPin)<512)) goto wait;
// 
//  lcd.setCursor(0, 1);  
//  lcd.print("Now rebooting...    ");   
//  delay (1000);     
//
//  setup (); 
//} 
// 
