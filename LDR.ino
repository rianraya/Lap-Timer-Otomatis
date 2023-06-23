  /*********************************************************************
        LIBRARY
*********************************************************************/
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>



/*********************************************************************
        INITIAL CONFIG
*********************************************************************/
    /*==========
      BUTTON
    ============*/
    const int btnRIGHT = BUTTON_RIGHT;
    const int btnUP = BUTTON_UP;
    const int btnDOWN = BUTTON_DOWN;
    const int btnLEFT = BUTTON_LEFT;
    const int btnSELECT = BUTTON_SELECT;

    /*==========
      LED
    ============*/
    int greenLED = A3;
    int redLED = A4; //was blueLED
    // int redLED = A5;

    /*==========
      LCD & ICONS
    ============*/
    Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
    
    byte arrow[8] = {0x0,0x10,0x18,0x1c,0x18,0x10,0x0};
    byte pause[8] = {0x0,0x1b,0x1b,0x1b,0x1b,0x1b,0x0};
    byte reset[8] = {0x0,0x1f,0x1f,0x1f,0x1f,0x1f,0x0};
    byte menu[8] = {0x0,0x17,0x0,0x17,0x0,0x17,0x0};
    int Arrow = 0;
    int Pause = 1;
    int Reset = 2;
    int Menu = 3;
    #define RED 0x1
    #define YELLOW 0x3
    #define GREEN 0x2
    #define TEAL 0x6
    #define BLUE 0x4
    #define VIOLET 0x5
    #define WHITE 0x7

    /*==========
      METHOD : laserCheck()
    ============*/
    //long timeSinceActivity = 0;
    
    /*==========
      METHOD : read_lcd_buttons()
    ============*/
    int lcd_key = 0;
    int adc_key_in = 0;
    
    /*==========
      METHOD : stopwatch()
    ============*/
    int LDR1 = A1;
    int LDR2 = A2;
    int lightStatus = 0; //Variable that tells us how much light there is
    int laserNotAligned = 0; //Variable for saving the analog value when the laser is not aligned
    int laserAligned;//For the timer trigger
    int laserAlignedValue = 0; //Variable that tells us how much the photoresistors throw when the laser is aligned
    long firstLaserContact = 0; //Variable that measures time since laser is aligned
    boolean running = false; boolean paused;
    unsigned long startTime, elapsedTime, pauseTime;
    
    /*==========
      Additional
    ============*/
    int Mode; //Switch-Case property
    int buzzpin=2;


/*********************************************************************
        METHODS / FUNCTIONS
*********************************************************************/
int read_lcd_buttons() {
    return lcd.readButtons();
}

int read_LDR(){
    return analogRead(LDR2) + analogRead(LDR1);
}

bool laserCheck(bool generalLoop) {
    lcd.setBacklight(WHITE);
    lightStatus = analogRead(LDR2) + analogRead(LDR1);
    if (generalLoop == false){
        lcd.clear();
        lcd.print("Place Module");
        while (lcd_key != btnSELECT){
            lightStatus = analogRead(LDR1) + analogRead(LDR2);
            laserNotAligned = lightStatus;
            lcd_key = read_lcd_buttons();
            delay(10);
        }
    lcd.clear();
    lcd.print("Point Laser");
    delay(1500);
    firstLaserContact = millis();
    while (millis() - firstLaserContact < 1500){
        lightStatus = analogRead(LDR1) + analogRead(LDR2);
        if (lightStatus > (laserNotAligned + 150)) {
            lcd.clear();
            lcd.print("Aligned");
            Serial.println(lightStatus);
            laserAligned=lightStatus;
            lcd.setBacklight(YELLOW);
        }else{
            firstLaserContact = millis();
            lcd.clear();
            lcd.print("Misaligned");
            Serial.println(lightStatus);
            lcd.setBacklight(RED);
        }
        delay(290);
    }
    lcd.clear();
    }else if (generalLoop == true) {
        if (lightStatus < (laserNotAligned + 150)){
            firstLaserContact = millis();
            while ((millis() - firstLaserContact) < 500){
                lightStatus = analogRead(LDR1) + analogRead(LDR2);
                if (lightStatus >= (laserNotAligned + 250)) {
                    lcd.clear();
                    lcd.print("Aligned");
                    Serial.println(lightStatus);
                    laserAligned=lightStatus;
                    lcd.setBacklight(YELLOW);
                } else {
                    firstLaserContact = millis();
                    lcd.clear();
                    lcd.print("Misaligned");
                    Serial.println(lightStatus);
                    lcd.setBacklight(RED);
                }
                delay(300);
            }
            lcd.clear();
            lcd.setBacklight(WHITE);
            lcd.write(Arrow);
            lcd.print("Manual");
            lcd.setCursor(1, 1);
            lcd.print("Auto");
        }
    }
}

void stopwatch(int mode){
    lcd.clear();
    lcd_key=0;
    delay(200);
    lcd.setCursor(6, 1);
    lcd.write(Reset);
    lcd.setCursor(9, 1);
    lcd.write(Arrow);
    
    //MANUAL MODE
    while(mode==0){
        //LCD Mechanism
        if(running==false&&paused==true&&startTime!=0)
            lcd.setBacklight(RED);
        else
            lcd.setBacklight(WHITE);
            
        //Stopwatch Mechanism
        if (running){
            elapsedTime = millis() - startTime;
        }
        int minutes = (int)(elapsedTime / 60000) % 60;
        int seconds = (int)(elapsedTime / 1000) % 60;
        int milliseconds = (int)(elapsedTime % 1000) / 10;
        
        lcd.setCursor(5, 0);
        lcd.print(minutes);
        lcd.print(":");
        if (seconds < 10) {
            lcd.print("0");
        }
        lcd.print(seconds);
        lcd.print(":");
        if (milliseconds < 10) {
            lcd.print("0");
        }
        lcd.print(milliseconds);
        
        //Buttons Mechanism
        if(read_lcd_buttons()==btnSELECT){
            delay(200);
            if (running == false && startTime == 0){ //to Start
                lcd.setBacklight(YELLOW);
                delay(100);
                lcd.setBacklight(WHITE);
                tone(buzzpin,523,200);
                startTime = millis();
                paused = false;
                running = true;
                lcd.setCursor(6, 1);
                lcd.write(Reset);
                lcd.setCursor(9, 1);
                lcd.write(Pause);
            }
            else if(running == false && paused == true){ //to resume
                tone(buzzpin,523,200);
                lcd.setBacklight(WHITE);
                startTime=millis() - pauseTime;
                paused = false;
                running = true;
                lcd.setCursor(6, 1);
                lcd.write(Reset);
                lcd.setCursor(9, 1);
                lcd.write(Pause);
            }
            else if(running == true && paused == false){ //to pause
                tone(buzzpin,523,200);
                lcd.setBacklight(RED);
                pauseTime=millis() - startTime;
                paused = true;
                running = false;
                lcd.setCursor(6, 1);
                lcd.write(Reset);
                lcd.setCursor(9, 1);
                lcd.write(Arrow);
            }
        }
        
        else if(read_lcd_buttons() == btnRIGHT){ //to reset
            lcd.setBacklight(TEAL);
            delay(150);
            lcd.setBacklight(WHITE);
            tone(buzzpin,659,200);
            running = false;paused=true;
                
            lcd.clear();
            lcd.setCursor(5, 0);
            lcd.print("0");
            lcd.print(":");
            lcd.print("0");
            lcd.print("0");
            lcd.print(":");
            lcd.print("0");
            lcd.print("0");
            lcd.setCursor(6, 1);
            lcd.write(Reset);
            lcd.setCursor(9, 1);
            lcd.write(Arrow);
            startTime=0;elapsedTime=0;
        }
        
        else if(read_lcd_buttons()==btnLEFT){ //to Menu
            lcd.setBacklight(WHITE);
            lcd.clear();
            lcd.write(Arrow);
            lcd.print("Manual");
            lcd.setCursor(1, 1);
            lcd.print("Auto");
            mode=-1;Mode=0;
        }
    }
    
    //AUTO MODE
    while(mode==1){
        //LCD Mechanism
        if(running==false&&paused==true&&startTime!=0)
            lcd.setBacklight(RED);
        else if(running==false&&paused==false&&startTime!=0)
            lcd.setBacklight(YELLOW);
        else
            lcd.setBacklight(WHITE);
            
        //Stopwatch Mechanism
        if (running){
            elapsedTime = millis() - startTime;
        }
        int minutes = (int)(elapsedTime / 60000) % 60;
        int seconds = (int)(elapsedTime / 1000) % 60;
        int milliseconds = (int)(elapsedTime % 1000) / 10;
        
        lcd.setCursor(5, 0);
        lcd.print(minutes);
        lcd.print(":");
        if (seconds < 10) {
            lcd.print("0");
        }
        lcd.print(seconds);
        lcd.print(":");
        if (milliseconds < 10) {
            lcd.print("0");
        }
        lcd.print(milliseconds);
        
        //Laser Mechanism
        if(read_LDR() <= (laserAligned - 150)){
            //delay(100);
            if (running == false && startTime == 0){ //to Start
                lcd.setBacklight(YELLOW);
                delay(20);
                lcd.setBacklight(WHITE);
                tone(buzzpin,523,200);
                lcd.setBacklight(WHITE);
                startTime = millis();
                paused = false;
                running = true;
                lcd.setCursor(6, 1);
                lcd.write(Reset);
                lcd.setCursor(9, 1);
                lcd.write(Pause);
                delay(100);
            }
            else if(running == true && paused == false){ //to stop
                tone(buzzpin,523,200);
                lcd.setBacklight(YELLOW);
                pauseTime=millis() - startTime;
                paused = false;
                running = false;
                lcd.setCursor(6, 1);
                lcd.write(Reset);
                lcd.setCursor(9, 1);
                lcd.write(Arrow);
            }
        }
        
        else if(read_lcd_buttons() == btnRIGHT){ //to reset
            lcd.setBacklight(TEAL);
            delay(150);
            lcd.setBacklight(WHITE);
            tone(buzzpin,659,200);
            running = false;paused=true;
            
            lcd.clear();
            lcd.setCursor(5, 0);
            lcd.print("0");
            lcd.print(":");
            lcd.print("0");
            lcd.print("0");
            lcd.print(":");
            lcd.print("0");
            lcd.print("0");
            lcd.setCursor(6, 1);
            lcd.write(Reset);
            lcd.setCursor(9, 1);
            lcd.write(Arrow);
            startTime=0;elapsedTime=0;
        }
        
        else if(read_lcd_buttons()==btnLEFT){ //to Menu
            lcd.setBacklight(WHITE);
            lcd.clear();
            lcd.write(Arrow);
            lcd.print("Manual");
            lcd.setCursor(1, 1);
            lcd.print("Auto");
            mode=-1;Mode=0;
        }
    }
}

void mainmenu(){
    lcd_key = read_lcd_buttons();
    switch (lcd_key){
        case btnUP:
            lcd.clear();
            lcd.write(0);
            lcd.print("Manual");
            lcd.setCursor(1, 1);
            lcd.print("Auto");
            Mode=0;
            break;
        
        case btnDOWN:
            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Manual");
            lcd.setCursor(0, 1);
            lcd.write(Arrow);
            lcd.print("Auto");
            Mode=1;
            break;
    }
}



/*********************************************************************
        SETUP
*********************************************************************/
void setup(){
    Serial.begin(9600);
    
    lcd.begin(16, 2);
    pinMode(greenLED, OUTPUT);
    //pinMode(blueLED, OUTPUT);
    pinMode(redLED, OUTPUT);
    lcd.createChar(Arrow, arrow);
    lcd.createChar(Pause, pause);
    lcd.createChar(Reset, reset);
    lcd.createChar(Menu, menu);
    
    analogWrite(greenLED,255);
    lcd.clear();
    lcd.setBacklight(WHITE);
    lcd.setCursor(4, 0);
    lcd.print("Welcome");
    tone(buzzpin,523,200);
    delay(500);
    tone(buzzpin,659,200);
    delay(200);
    
    lcd.clear();
    laserCheck(false);
    delay(500);
    
    lcd.clear();
    lcd.setBacklight(WHITE);
    lcd.print("Starting..");
    delay(1750);
    
    lcd.clear();
    lcd.write(Arrow);
    lcd.print("Manual");
    lcd.setCursor(1, 1);
    lcd.print("Auto");
    Mode=0;
}



/*********************************************************************
        LOOP
*********************************************************************/
void loop(){
    laserCheck(true);
    mainmenu();
    if(read_lcd_buttons()==btnSELECT){
        stopwatch(Mode);
    }
} 
