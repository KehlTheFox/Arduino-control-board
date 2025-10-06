/*------------------------------------------------------------------------------------
       74HC165 pins                        74HC595 pins

A3 PL LTC-|||-16 VCC           |           1D-|||-VCC
A5 CP CLK-|||-15 CE CLN A4     |           2D-|||-0D
       D4-|||-D3               |           3D-|||-DS DAT1
       D5-|||-D2               |           4D-|||-OE GND
       D6-|||-D1               |           5D-|||-ST_CP LTC1
       D7-|||-D0               |           6D-|||-SH_CP CLK1
      INV-|||-DATIN            |           7D-|||-MR VCC
      GND-|||-DAT A2           |          GND-|||-Q7, to next Shift_register DS


--------------
  DATAIN0    DATAIN1
 *||||||||   ||||||||*
             57016234
    X                  - It doesn't work in the prototype (MAIN_BOARD_X) due to a defect on the board.
--------------
  DATAIN0
  NUl - NUl
  NUl - NUl
  NUl - DEAD PIN
  NUl - NUl
  NUl - NUl
  NUl - NUl
  NUl - NUl
  NUl - NUl

  DATAIN1
  4 - LEFT TURN
  3 - RIGHT TURN
  2 - ALARM 
  6 - Low lamp
  1 - High lamp
  0 - Korn lamp 
  7 - Fog lamp
  5 - DRL
--------------

------------------------------------------------------------------------------------*/

//Global setup:
//Timers :
unsigned long Timer0;  //DEBUGGING
unsigned long Timer1;  // Action ALARM
unsigned long Timer2;  // Action Turn Left
unsigned long Timer3;  // Action Turn Right
unsigned long Timer4;  // Read ALARM
unsigned long Timer5;  // Read Turn Left
unsigned long Timer6;  // Read Turn Right
unsigned long Timer7;  // Low lamp
unsigned long Timer7_1;// High lamp
unsigned long Timer8;  // Korn lamp
unsigned long Timer9;  // Fog lamp
unsigned long Timer10; // DRL
//Input DATA :
byte DATAIN0 = (0b00000000);
byte DATAIN1 = (0b00000000);  // if bit is 0 = no input signal
//Output DATA :
byte DATAOUT0 = (0b11111111);
byte DATAOUT1 = (0b11111111);
byte DATAOUT2 = (0b11111111);  // if bit is 1 = no output signal, relay is OFF
//Variables
bool FlagL = 0;      // Flag of LEFT turn (ACTION)
bool FlagR = 0;      // Flag of RIGHT turn (ACTION)
bool FlagALARM = 0;  // Flag of ALARM BUTTON (ACTION)
bool BtnL = 0;       // Left turn button (READ)
bool BtnR = 0;       // Right turn button (READ)
bool BtnALARM = 0;   // Alarm button (READ)
bool STATE = 1;      // Variable for TURN_SIGNAL_ACTION
bool LowLamp = 0;    
bool HighLamp = 0;
bool Kornlamp = 0;
bool Foglamp = 0;
bool DRL = 0;
int Init = 0;        // Initialization process 
int RS = 0;          // Flag for delayed start of relay modules
int Count = 0;

//Define pins :
//Input :
#define CLK A5
#define CLN A4
#define LTC A3
#define DAT A2
//Output :
#define DAT1 9
#define LTC1 10
#define CLK1 11

void setup() {
  // Setup Serial Monitor
  Serial.begin(9600);
  // Setup 74HC165 connections
  pinMode(LTC, OUTPUT);
  pinMode(CLN, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DAT, INPUT);

  // Setup 74HC595 connections
  pinMode(LTC1, OUTPUT);
  pinMode(CLK1, OUTPUT);
  pinMode(DAT1, OUTPUT);

  pinMode(12, INPUT);  //Reserve
  pinMode(13, OUTPUT);  //Turn ON Relay
}

void loop() {
  // FIRST STEP - Input data from input stage
  DATA_INPUT();

  // SECOND STEP - Logical operations and data manipulation
  TURN_SIGNAL_READ();
  TURN_SIGNAL_ACTION();

  LOW_LAMP();

  HIGH_LAMP();

  KORN_LAMP();

  FOG_LAMP();

  DAY_RIDE_LIGHT ();

  //THIRD STEP - Sending data to the output stage
  DATA_OUTPUT();

  // Blocks of code that are used only once.
  RELAY_START(); // Don't disable this if your relay modules have premature tripping on all lines when power is applied.
  INIT(); // If you don't need this feature, you can safely disable it.
}

void DATA_INPUT() {
  // Algorithm for reading from the input stage
  // Preparing to read data from registers
  digitalWrite(LTC, 0); // Open LTC. Receiving data to the register from his pins.
  digitalWrite(LTC, 1); // Close LTC. Storing data inside a register
  digitalWrite(CLK, 1); // Up CLK for prepare 
  digitalWrite(CLN, 0); // Start reading...
  delay(1);
  DATAIN0 = shiftIn(DAT, CLK, LSBFIRST); // Sending data from register to variable
  delay(1);
  DATAIN1 = shiftIn(DAT, CLK, LSBFIRST); // And again
  delay(1);
  digitalWrite(CLN, 1); // Stop reading. 
}

void DATA_OUTPUT() {
  // Algorithm for writing to the output stage
  
  digitalWrite(LTC1, 0); // Open LTC. Ready to sending
  delay(1);
  shiftOut(DAT1, CLK1, MSBFIRST, DATAOUT0); // Sending data to register from variable
  delay(1);
  shiftOut(DAT1, CLK1, MSBFIRST, DATAOUT1); // And again...
  delay(1);
  shiftOut(DAT1, CLK1, MSBFIRST, DATAOUT2); // And again
  delay(1);
  digitalWrite(LTC1, 1); // Stop sending. Close LTC
}

void TURN_SIGNAL_READ() {
  // An algorithm for determining which turn signal is turned on (the physical lever in the car) and how exactly it is turned on.
  // - - - - - - - - - - - ALARM - - - - - - - - - - -
  BtnALARM = bitRead(DATAIN1, 2);
  if (BtnALARM == 1 && FlagALARM == 0 && millis() - Timer4 > 250) {
    Timer4 = millis();
    FlagALARM = 1;
  }

  if (BtnALARM == 0 && FlagALARM == 1 && millis() - Timer4 > 1100) {
    Timer4 = millis();
    FlagALARM = 0;
    bitWrite(DATAOUT0, 0, 1);
    bitWrite(DATAOUT0, 1, 1);
  }


  if (FlagALARM == 0) {
    // - - - - - - - - - - - LEFT - - - - - - - - - - -
    BtnL = bitRead(DATAIN1, 4);
    if (BtnL == 1 && FlagL == 0 && millis() - Timer5 > 250) {
      Timer5 = millis();
      FlagL = 1;
    }

    if (BtnL == 0 && FlagL == 1 && millis() - Timer5 > 1100) {
      Timer5 = millis();
      FlagL = 0;
      bitWrite(DATAOUT0, 0, 1);
    }

    // - - - - - - - - - - - RIGHT - - - - - - - - - - -
    BtnR = bitRead(DATAIN1, 3);
    if (BtnR == 1 && FlagR == 0 && millis() - Timer6 > 250) {
      Timer6 = millis();
      FlagR = 1;
    }

    if (BtnR == 0 && FlagR == 1 && millis() - Timer6 > 1100) {
      Timer6 = millis();
      FlagR = 0;
      bitWrite(DATAOUT0, 1, 1);
    }
  }
}

void TURN_SIGNAL_ACTION() {

  // Algorithm for blinking the necessary lights when the turn signals or the emergency button are on
  // - - - - - - - - - - - ALARM - - - - - - - - - - -
  if (FlagALARM == 1) {
    if (millis() - Timer1 > 650) {
      Timer1 = millis();
      STATE = !STATE;
      bitWrite(DATAOUT0, 0, STATE);
      bitWrite(DATAOUT0, 1, STATE);
    }
  }


  // - - - - - - - - - - - LEFT - - - - - - - - - - -
  if (FlagALARM == 0 && FlagL == 1) {
    if (millis() - Timer2 > 650) {
      Timer2 = millis();
      STATE = !STATE;
      bitWrite(DATAOUT0, 0, STATE);
    }
  }
  // - - - - - - - - - - - RIGHT - - - - - - - - - - -
  if (FlagALARM == 0 && FlagR == 1) {
    if (millis() - Timer3 > 650) {
      Timer3 = millis();
      STATE = !STATE;
      bitWrite(DATAOUT0, 1, STATE);
    }
  }
}

void LOW_LAMP() {
  // Algorithm for switching ON low beam headlights.
  LowLamp = bitRead(DATAIN1, 6); // Checking the pin status
  
  // Turn ON
  if (LowLamp == 1 && HighLamp == 0 && millis() - Timer7 > 500) {
    Timer7 = millis();
    bitWrite(DATAOUT0, 2, 0);
    bitWrite(DATAOUT0, 7, 0);
  }

  // Turn OFF when LowLamp is ON and High Lamp is ON too. High Lamp has higher priority
  if (LowLamp == 1 && HighLamp == 1 && millis() - Timer7 > 500) {
    Timer7 = millis();
    bitWrite(DATAOUT0, 2, 1);
    bitWrite(DATAOUT0, 7, 1);
  }

  // Turn OFF
  if (LowLamp == 0 && millis() - Timer7 > 500) {
    Timer7 = millis();
    bitWrite(DATAOUT0, 2, 1);
    bitWrite(DATAOUT0, 7, 1);
  }

}

void HIGH_LAMP() {
  // Algorithm for switching ON high beam headlights.
  HighLamp = bitRead(DATAIN1, 1); // Checking the pin status
  
  // Turn ON
  if (HighLamp == 1 && millis() - Timer7_1 > 500) {
    Timer7_1 = millis();
    bitWrite(DATAOUT0, 3, 0);
    bitWrite(DATAOUT0, 6, 0);
  }

  // Turn OFF
  if (HighLamp == 0 && millis() - Timer7_1 > 500) {
    Timer7_1 = millis();
    bitWrite(DATAOUT0, 3, 1);
    bitWrite(DATAOUT0, 6, 1);
  }
}

void KORN_LAMP() {
  // Algorithm for switching ON parking lights, anti-collision lights.
  Kornlamp = bitRead(DATAIN1, 0); // Checking the pin status

  //Turn ON
  if (Kornlamp == 1 && millis() - Timer8 > 500) {
    Timer8 = millis();
    bitWrite(DATAOUT0, 4, 0);
    bitWrite(DATAOUT0, 5, 0);
    bitWrite(DATAOUT2, 0, 0);
    bitWrite(DATAOUT2, 1, 0);
    bitWrite(DATAOUT2, 7, 0);
    bitWrite(DATAOUT2, 2, 0);
  }

  // Turn OFF
  if (Kornlamp == 0 && millis() - Timer8 > 500) {
    Timer8 = millis();
    bitWrite(DATAOUT0, 4, 1);
    bitWrite(DATAOUT0, 5, 1);
    bitWrite(DATAOUT2, 0, 1);
    bitWrite(DATAOUT2, 1, 1);
    bitWrite(DATAOUT2, 7, 1);
    bitWrite(DATAOUT2, 2, 1);
  }
}

void FOG_LAMP() {
  // Algorithm for turning ON fog lights.
  Foglamp = bitRead(DATAIN1, 7); // Checking the pin status

  // Turn ON
  if (Foglamp == 1 && millis() - Timer9 > 500) {
    Timer9 = millis();
    bitWrite(DATAOUT2, 3, 0);
    bitWrite(DATAOUT2, 6, 0);
  }

  // Turn OFF
  if (Foglamp == 0 && millis() - Timer9 > 500) {
    Timer9 = millis();
    bitWrite(DATAOUT2, 3, 1);
    bitWrite(DATAOUT2, 6, 1);
  }
}

void DAY_RIDE_LIGHT () {
  // Algorithm for switching ON daytime running lights.
  DRL = bitRead(DATAIN1, 5); // Checking the pin status

  // Turn ON
  if (DRL == 1 && millis() - Timer10 > 500) {
    Timer10 = millis();
    bitWrite(DATAOUT2, 4, 0);
    bitWrite(DATAOUT2, 5, 0);
  }

  // Turn OFF
  if (DRL == 0 && millis() - Timer10 > 500) {
    Timer10 = millis();
    bitWrite(DATAOUT2, 4, 1);
    bitWrite(DATAOUT2, 5, 1);
  }
}

void RELAY_START () {
  /*
  Delayed switching on of relay module. 
  It is necessary if relay module with 0 == ON and 1 == OFF are used. 
  By default, the registers hold 0 on their pins and when the relay module are switched on, ALL lines are switched on at the same moment. 
  Such relay module need to be supplied with 1 for the "off" state, 
  But Arduino or ESP32 require some time to switch on and raise 1 on the register pins. 
  This block of code allows you to supply power to the relay module AFTER the Arduino is activated and 1 is raised on the register pins.
  */
   
  if ( RS == 0 ){
    digitalWrite (13, 1);
    delay(10);
    DATA_OUTPUT();
    delay(10);
    RS = 1;
  }
}

void INIT (){
  /* 
  Basically, it's just *pretty blinking lights*, when you look at it from a software code perspective.
  
  This code is NOT useful for the sketch and doesn't do anything important or necessary. 
  If you don't need this feature, you can safely disable it.
  
  However, the car has factory systems, and according to the NISSAN manual, 
  you CANNOT start the engine during the first 5 seconds of the key being in the *On* position. 
  During this time, the car's ECU performs a selftest, the fuel pump raises the fuel pressure to operating pressure, 
  and the car do other prepares to start the engine.
  
  In this block of code, all the timing is chosen to occupy those 5 seconds while the car is preparing to start the engine. 
  During this time, the taillights blink beautifully, just like on modern cars.
  */

  if ( RS == 1 && Init == 0 && millis() - Timer0 > 500 ) {
    Timer0 = millis();
    int Period = 200;
    int Period1 = 950;
    
    delay(1);
    Serial.print("Запуск и инициализация систем автомобиля...");
    delay(1);

    bitWrite(DATAOUT0, 4, 0);
    bitWrite(DATAOUT0, 5, 0);
    delay(Period);

    bitWrite(DATAOUT2, 7, 0);
    bitWrite(DATAOUT2, 2, 0);
    DATA_OUTPUT();
    delay(Period);

    bitWrite(DATAOUT2, 0, 0);
    bitWrite(DATAOUT2, 1, 0);
    DATA_OUTPUT();
    delay(Period1);

    bitWrite(DATAOUT2, 7, 1);
    bitWrite(DATAOUT2, 2, 1);
    DATA_OUTPUT();
    delay(Period);

    bitWrite(DATAOUT2, 0, 1);
    bitWrite(DATAOUT2, 1, 1);
    DATA_OUTPUT();
    delay(Period1);

    bitWrite(DATAOUT2, 7, 0);
    bitWrite(DATAOUT2, 2, 0);
    DATA_OUTPUT();
    delay(Period);

    bitWrite(DATAOUT2, 0, 0);
    bitWrite(DATAOUT2, 1, 0);
    DATA_OUTPUT();
    delay(Period1);

    bitWrite(DATAOUT2, 7, 1);
    bitWrite(DATAOUT2, 2, 1);
    DATA_OUTPUT();
    delay(Period);

    bitWrite(DATAOUT2, 0, 1);
    bitWrite(DATAOUT2, 1, 1);
    DATA_OUTPUT();
    delay(Period1);

    bitWrite(DATAOUT2, 7, 0);
    bitWrite(DATAOUT2, 2, 0);
    DATA_OUTPUT();
    delay(Period);

    bitWrite(DATAOUT2, 0, 0);
    bitWrite(DATAOUT2, 1, 0);
    DATA_OUTPUT();
    delay(Period1);

    bitWrite(DATAOUT2, 7, 1);
    bitWrite(DATAOUT2, 2, 1);
    DATA_OUTPUT();
    delay(Period);

    bitWrite(DATAOUT2, 0, 1);
    bitWrite(DATAOUT2, 1, 1);
    DATA_OUTPUT();
    delay(Period);

    Init = 1;

    delay(1);
    Serial.println(" успешно");

  }

}