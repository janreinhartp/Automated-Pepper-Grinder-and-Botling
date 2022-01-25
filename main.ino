// Declaration of Libraries
    //LCD
    #include <Wire.h> 

    #include <LiquidCrystal_I2C.h>
    LiquidCrystal_I2C lcd(0x27,20,4); 

    //Button
    #include <Button.h>

    //Encoder
    #include <ClickEncoder.h>

    //Timer 1 for encoder
    #include <TimerOne.h>

    //Save Function
    #include <EEPROMex.h>

// Declaration of LCD Variables
    int refreshScreen=0;

    const int numOfMainScreens = 3;
    const int numOfSettingScreens = 3;
    const int numOfmotorSpeed = 3;
    const int numOfTestMenu = 4;
    int currentScreen = 0;
    int currentSettingScreen = 0;
    int currentmotorSpeed = 0;
    int currentTestMenuScreen = 0;
    String screens[numOfMainScreens][2] = {{"Settings","Click to Edit"}, {"Run Auto", "Enter to Run"},{"Test Machine", "Enter to Test"}};
    String settings[numOfSettingScreens][2] = {{"Grinding Time","Mins"}, {"Motor Speed", "Click to Edit"},{"Save Settings", "Enter to Save"}};
    String TestMenuScreen[numOfTestMenu] = {"Test Grinder","Test Conveyor","Test Volumetric","Back to Main Menu"};

    String motorSpeed[numOfSettingScreens][2] = {{"Conveyor Speed","Hz"}, {"Volumetric Speed", "Hz"},{"Save Settings", "Enter to Save"}};

    double parameters[numOfSettingScreens]={0,0,0};
    double parametersMaxValue[numOfSettingScreens]={60,60,60};

    double parametersMotor[numOfSettingScreens]={0,0};
    double parametersMaxValueMotor[numOfSettingScreens]={60,60};

    double hz_multiplier = 255 / 60;

    bool settingsFlag = false;
    bool settingsEditFlag = false;
    bool motorSpeedFlag = false;
    bool motorEditSpeedFlag = false;
    bool testMenuFlag = false;
    bool runAutoFlag = false;
    bool runManualFlag = false;

    byte enterChar[] = {
    B10000,
    B10000,
    B10100,
    B10110,
    B11111,
    B00110,
    B00100,
    B00000
    };

    byte fastChar[] = {
    B10111,
    B10101,
    B10111,
    B00000,
    B00000,
    B00100,
    B01110,
    B11111
    };

// Declaration of Variables

    //Rotary Encoder Variables
    boolean up = false;
    boolean down = false;
    boolean middle = false;

    ClickEncoder *encoder;
    int16_t last, value;

    //Fast Scroll
    bool fastScroll = false;

    //Relay Variables
    int relay_grinder = 8;
    int relay_volumetric = 9;
    int relay_conveyor = 10;

    int relay_FWD = 12;
    int PWM_Speed = 11;

    //Sensor Variables
    int volumetric_sen = 3;
    int jar_sen = 2;

    volatile int volumetric_state = false;
    volatile int jar_state = false;

    int test_Speed = 125;

    //Save to EEPROMex Adresses
    int grindTimeAdd = 0;
    int conveyorHZAdd = 10;
    int volumetricHZAdd = 20;

    double grindTime = 0;
    double conveyorHZ = 0;
    double volumetricHZ = 0;
    bool volumetric_InitialMove = false;
    bool conveyor_InitialMove = false;

// Declaration of Function
    //Functions for Rotary Encoder
    void timerIsr(){
    encoder->service();
    }

    void readRotaryEncoder(){
    value += encoder->getValue();
    
    if (value/2 > last) {
        last = value/2;
        down = true;
   
    }else   if (value/2 < last) {
        last = value/2;
        up = true;
       
    }
    }

    bool testconveyorRunFlag = false;
    void testrunConveyor(){
        if (testconveyorRunFlag == true)
        {
            if (digitalRead(jar_sen) == 0)
            {
               testconveyorRunFlag = false;
               refreshScreen = 1;
            }
            digitalWrite(relay_conveyor, LOW);
        }
        else
        {
            digitalWrite(relay_conveyor, HIGH);
        }
        
    }

    bool testvolumetricRunFlag = false;
    void testrunvolumetric(){
            if (testvolumetricRunFlag == true)
            {
                if (digitalRead(volumetric_sen) == 0)
                {
                testvolumetricRunFlag = false;
                refreshScreen = 1;
                }
                digitalWrite(relay_volumetric, LOW);
            }
            else
            {
                digitalWrite(relay_volumetric, HIGH);
            }
        
    }

    bool testGrinderRunFlag = false;

    void testrunGrinder(){
        if (testGrinderRunFlag == true)
        {
            digitalWrite(relay_grinder, LOW);
        }
        else
        {
            digitalWrite(relay_grinder, HIGH);
        }
    }

    void stopAllTest(){
        testGrinderRunFlag = false;
        testconveyorRunFlag = false;
        testvolumetricRunFlag = false;
        digitalWrite(relay_grinder, HIGH);
        digitalWrite(relay_conveyor, HIGH);
        digitalWrite(relay_volumetric, HIGH);
        
    }

   

    bool grinderRunAutoFlag, pauseAfterGrindRunAutoFlag, volumetricRunAutoFlag, conveyorRunAutoFlag = false;

    void runAuto(){
        if (grinderRunAutoFlag == true)
        {
            grinderRunAuto();
        }
        else if (pauseAfterGrindRunAutoFlag == true)
        {
            Serial.println("Pause after Grind");
        }
        else if (volumetricRunAutoFlag == true)
        {
            volumetricRunAuto();
        }
        else if (conveyorRunAutoFlag == true)
        {
            conveyorRunAuto();
        }
    }

    unsigned long currentMillis = 0;
    unsigned long previousMillis = 0;
    unsigned long minRemaining = 0;
    unsigned long secRemaining = 0;
    unsigned long timerGrinder = 0;

    void grinderRunAuto(){
        if(currentMillis - previousMillis <= timerGrinder){
            Serial.println("Timer on");
            digitalWrite(relay_grinder, LOW);

            secRemaining = (((timerGrinder- (currentMillis - previousMillis))/ 1000));
            minRemaining = (((timerGrinder- (currentMillis - previousMillis))/ 1000)/60);
            Serial.print("Remaining Time:");
            Serial.println(secRemaining);
        }else{
            digitalWrite(relay_grinder, HIGH);
            pauseAfterGrindRunAutoFlag = true;
            grinderRunAutoFlag = false;
            Serial.println("Timer off");
            refreshScreen = 1;
            delay(5000);
            pauseAfterGrindRunAutoFlag = false;
            conveyorRunAutoFlag = true;
            conveyor_InitialMove = true;
        }
    }

    

     void TestSensorPins(){
            Serial.println(volumetric_state);
            Serial.println(jar_state);
    }

    void readSensors(){
        jar_state = digitalRead(jar_sen);
        volumetric_state = digitalRead(volumetric_sen);
    }

  
    // Volumetric Run Function : Check if jar is position and Move the volumetric cup until ground pepper is dispense
    void volumetricRunAuto(){

        analogWrite(PWM_Speed, parametersMotor[1] * hz_multiplier);
        digitalWrite(relay_conveyor, HIGH);
        
        if (jar_state == 0) // Check if the glass jar is position Properly
        {
            if (volumetric_InitialMove == true) // Check for volumetric position move if sensor is hit
            {
                Serial.println("Conve Initial Move");
                if (volumetric_state == 0) // Check if the volumetric is finish dispensing
                {
                    digitalWrite(relay_volumetric, LOW);
                    digitalWrite(relay_FWD, LOW);
                    volumetric_InitialMove = false;
                    delay(300);
                }else{
                    volumetric_InitialMove = false;
                    delay(300);
                }
            }
            else 
            {
                if (volumetric_state == 0) // Check if the volumetric is finish dispensing
                {
                    digitalWrite(relay_volumetric, HIGH);
                    digitalWrite(relay_FWD, HIGH);
                    conveyor_InitialMove = true;
                    conveyorRunAutoFlag = true;
                    volumetricRunAutoFlag = false;
                    delay(2000);

                }
                else
                {
                    digitalWrite(relay_volumetric, LOW);
                    digitalWrite(relay_FWD, LOW);
                }
            }
            
        }
        else // If False jar read conveyor will run
        {
            conveyor_InitialMove = true;
            conveyorRunAutoFlag = true;
            volumetricRunAutoFlag = false;
            volumetric_InitialMove = false;
        }
        
    }

    
    void conveyorRunAuto(){
        analogWrite(PWM_Speed, parametersMotor[0] * hz_multiplier);
        digitalWrite(relay_volumetric, HIGH);
        if (conveyor_InitialMove == true)
        {
            if (jar_state == 0)
            {
                Serial.println("Conve Initial Move - Jar Detected");

                digitalWrite(relay_conveyor, LOW);
                digitalWrite(relay_FWD, LOW);
                digitalWrite(relay_volumetric,HIGH);
               // conveyor_InitialMove = false;
            }
            else
            {
                Serial.println("Conve Initial Move - Not Jar Detected");
                digitalWrite(relay_conveyor, LOW);
                digitalWrite(relay_FWD, LOW);
                conveyor_InitialMove = false;
            }
        }
        else
        { 
            
            if (jar_state == 0)
            {
                Serial.println("Conve Move - Jar Detected");
                digitalWrite(relay_conveyor, HIGH);
                digitalWrite(relay_FWD, HIGH);
                stopAllMotor();
                conveyorRunAutoFlag = false;
                volumetric_InitialMove = true;
                volumetricRunAutoFlag = true;

            }
            else
            {
                Serial.println("Conve Move - Jar Detected");
                digitalWrite(relay_conveyor, LOW);
                digitalWrite(relay_FWD, LOW);
            }
        }
    }
    

    void stopAllMotor(){
        digitalWrite(relay_grinder, HIGH);
        digitalWrite(relay_volumetric, HIGH);
        digitalWrite(relay_conveyor, HIGH);
        digitalWrite(relay_FWD, HIGH);
    }

    unsigned long currentTime = millis();
    const unsigned long eventInterval = 1000;
    unsigned long previousTime = 0;

    void refreshScreensEvery1Second(){
        currentTime = millis();
        if (currentTime - previousTime >= eventInterval) {
            
            refreshScreen = 1;
        /* Update the timing for the next time around */
            previousTime = currentTime;
        }
    }


 

void setup()
{
    //save_setting();
    //Encoder Setup
    encoder = new ClickEncoder(7,6, 5);
    encoder->setAccelerationEnabled(false);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr); 
    last = encoder->getValue();

    //LCD Setup
    lcd.init();
    lcd.createChar(0, enterChar);
    lcd.createChar(1, fastChar);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("********************");
    lcd.setCursor(0,1);
    lcd.print("*");
    lcd.setCursor(3,1);
    lcd.print("GROUND PEPPER");
    lcd.setCursor(19,1);
    lcd.print("*");
    lcd.setCursor(0,2);
    lcd.print("*");
    lcd.setCursor(6,2);
    lcd.print("MACHINE");
    lcd.setCursor(19,2);
    lcd.print("*");

    lcd.setCursor(0,3);
    lcd.print("********************");

    //Serial Debug
    Serial.begin(9600);

    // Output void setup
    pinMode(relay_conveyor, OUTPUT);
    pinMode(relay_grinder, OUTPUT);
    pinMode(relay_volumetric, OUTPUT);
    pinMode(relay_FWD, OUTPUT);
    //Low Level Relay Reverse Input
    digitalWrite(relay_conveyor, HIGH);
    digitalWrite(relay_grinder, HIGH);
    digitalWrite(relay_volumetric, HIGH);
    digitalWrite(relay_FWD, HIGH);

    //PWM Output
    pinMode(PWM_Speed, OUTPUT);

    // Input void setup
    pinMode(volumetric_sen, INPUT_PULLUP);
    pinMode(jar_sen, INPUT_PULLUP);
    load_settings();
    delay(1000);
    refreshScreen = 1;
}

void loop()
{
    readSensors();
    currentMillis = millis();
    readRotaryEncoder(); 
    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) { 
    switch (b) { 
            case ClickEncoder::Clicked: 
                middle=true; 
            break;

            case ClickEncoder::DoubleClicked:
                Serial.println("ClickEncoder::DoubleClicked");
                refreshScreen=1;
                
                if (settingsFlag)
                {
                    if (fastScroll == false)
                    {
                        fastScroll = true;

                    }
                    else
                    {
                        fastScroll = false;
                    }

                }
                Serial.println(fastScroll);
                
            break;
    
    } 
    }


    //LCD Change Function and Values
    // To Right Rotary
    if (up == 1)
    {
    up= false;
    refreshScreen=1;
    if (settingsFlag == true)
    {
        if (motorSpeedFlag == true)
        {
            if (motorEditSpeedFlag == true)
            {
                if (parametersMotor[currentmotorSpeed] >= parametersMaxValueMotor[currentmotorSpeed]-1 ){
                    parametersMotor[currentmotorSpeed] = parametersMaxValueMotor[currentmotorSpeed];
                }else
                {
                    if (fastScroll == true)
                    {
                        parametersMotor[currentmotorSpeed] += 10;
                    }
                    else
                    {
                        parametersMotor[currentmotorSpeed] += 1;
                    }
                }
            }
            else
            {
                if (currentmotorSpeed == numOfmotorSpeed-1) {
                    currentmotorSpeed = 0;
                }else{
                    currentmotorSpeed++;
                }
            }
        }
        else
        {
            if (settingsEditFlag == true)
            {
                if (parameters[currentSettingScreen] >= parametersMaxValue[currentSettingScreen]-1 ){
                    parameters[currentSettingScreen] = parametersMaxValue[currentSettingScreen];
                }else
                {
                    if (fastScroll == true)
                    {
                        parameters[currentSettingScreen] += 1;
                    }
                    else
                    {
                        parameters[currentSettingScreen] += 0.1;
                    }
                }
            }
            else
            {
                if (currentSettingScreen == numOfSettingScreens-1) {
                    currentSettingScreen = 0;
                }else{
                    currentSettingScreen++;
                }
            }
        }
        
       
    }
    else if (testMenuFlag == true)
    {
         if (currentTestMenuScreen == numOfTestMenu-1) {
            currentTestMenuScreen = 0;
        }else{
            currentTestMenuScreen++;
        }
    }
    else
    {
        if (currentScreen == numOfMainScreens-1) {
            currentScreen = 0;
        }else{
            currentScreen++;
        }
    }
    }

    // To Left Rotary
    if (down == 1)
    {
    down = false;
    refreshScreen=1;
    if (settingsFlag == true)
    {
        if (motorSpeedFlag == true)
        {
             if (motorEditSpeedFlag == true)
            {

                if (parametersMotor[currentmotorSpeed] <= 0 ){
                    parametersMotor[currentmotorSpeed] = 0;
                }else
                {
                    if (fastScroll == true)
                    {
                        parametersMotor[currentmotorSpeed] -= 10;
                    }
                    else
                    {
                        parametersMotor[currentmotorSpeed] -= 1;
                    }
                }
            }
            else
            {
                if (currentmotorSpeed == 0) {
                    currentmotorSpeed = numOfmotorSpeed-1;
                }else{
                    currentmotorSpeed--;
                }
            }
        }
        else
        {
            if (settingsEditFlag == true)
            {

                if (parameters[currentSettingScreen] <= 0 ){
                    parameters[currentSettingScreen] = 0;
                }else
                {

                     if (fastScroll == true)
                    {
                        parameters[currentSettingScreen] -= 1;
                    }
                    else
                    {
                        parameters[currentSettingScreen] -= 0.1;
                    }
                }
            }
            else
            {
                if (currentSettingScreen == 0) {
                    currentSettingScreen = numOfSettingScreens-1;
                }else{
                    currentSettingScreen--;
                }
            }
        }
        

      
    }
    else if (testMenuFlag == true)
    {
         if (currentTestMenuScreen == 0) {
            currentTestMenuScreen = numOfTestMenu-1;
        }else{
            currentTestMenuScreen--;
        }
    }
    else
    {
        if (currentScreen == 0) {
            currentScreen = numOfMainScreens-1;
        }else{
            currentScreen--;
        }
    }
    }

    // Rotary Button Press
    if (middle==1)
    {
    middle = false;
    refreshScreen=1;

    if (settingsFlag == true)
    {
        if (currentSettingScreen == 2)
        {
            settingsFlag = false;
            save_setting();
        }
        else if (currentSettingScreen == 1){
            if (currentSettingScreen == 1 && motorSpeedFlag == false)
            {
                motorSpeedFlag = true;
            }
            else if (currentSettingScreen == 1 && motorSpeedFlag == true && currentmotorSpeed == 2)
            {
                motorSpeedFlag = false;
                save_setting();
            }
            else
            {
                if (motorEditSpeedFlag == true)
                {
                    motorEditSpeedFlag = false;
                }
                else
                {
                    motorEditSpeedFlag = true;
                }
                
            }
            
        }
        else
        {
            if (settingsEditFlag == true)
            {
                settingsEditFlag = false;
            }
            else
            {
                settingsEditFlag = true;
            }
        }
        }
        else if (runAutoFlag == true)
        {
        if (pauseAfterGrindRunAutoFlag == true)
        {
            pauseAfterGrindRunAutoFlag = false;
            conveyorRunAutoFlag = true;
            conveyor_InitialMove = true;
        }
        else
        {
            stopAllMotor();
            runAutoFlag = false; 
        }
        
       
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == 0)
            {
                if (testGrinderRunFlag == true)
                {
                    testGrinderRunFlag = false;
                    refreshScreen = 1;
                }
                else
                {
                    testGrinderRunFlag = true;
                    refreshScreen = 1;
                }
            
            }
            else if (currentTestMenuScreen == 1)
            {
                if (testconveyorRunFlag == true)
                {
                    testconveyorRunFlag = false;
                    refreshScreen = 1;
                }
                else
                {
                    testconveyorRunFlag = true;
                    refreshScreen = 1;
                }
            }
            else if (currentTestMenuScreen == 2)
            {
                if(digitalRead(jar_sen) == 0){
                    if (testvolumetricRunFlag == true)
                    {
                        testvolumetricRunFlag = false;
                        refreshScreen = 1;
                    }
                    else
                    {
                        testvolumetricRunFlag = true;
                        refreshScreen = 1;
                    }
                }else{
                    Serial.println("Jar is not Detected");
                }
            
            }
            else if (currentTestMenuScreen == 3)
            {
                testMenuFlag = false;
            }   
        }
        else
        {
            if (currentScreen == 0)
            {
                settingsFlag = true;
            }
            else if (currentScreen == 1)
            {
                runAutoFlag = true;
                timerGrinder = parameters[0] * 60000;
                grinderRunAutoFlag = true;
                previousMillis = currentMillis;
            }
            else if (currentScreen == 2)
            {
                testMenuFlag = true;
            }
        
            
        }
    }


    if (testMenuFlag == true)
    {
       // TestSensorPins();
        testrunConveyor();
        testrunvolumetric();
        testrunGrinder();
    }else if(runAutoFlag == true){
        runAuto();
        if (grinderRunAutoFlag == true)
        {
            refreshScreensEvery1Second();
        }
    }

    if(refreshScreen == 1){
        printScreens();
        refreshScreen = 0;
    }
}


//LCD Functions
void printScreens(){
    if (settingsFlag == true)
    {
        if (motorSpeedFlag == false)
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.print(settings[currentSettingScreen][0]);
            if(currentSettingScreen == 2){
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                lcd.print("Click to Save All");
            
            }else if(currentSettingScreen == 1)
            {
                lcd.setCursor(0,1);
                lcd.print("Edit Hz and Speed");
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                if(settingsEditFlag == false){
                    lcd.print("Click to Edit");   
                }else{
                    lcd.print("Click to Save");  
                }
                lcd.setCursor(19,3);

                if (fastScroll == true)
                {
                    lcd.write(1);
                }
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print(parameters[currentSettingScreen]);
                lcd.print(" ");
                lcd.print(settings[currentSettingScreen][1]);
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                if(settingsEditFlag == false){
                    lcd.print("Click to Edit");   
                }else{
                    lcd.print("Click to Save");  
                }

                lcd.setCursor(19,3);
                if (fastScroll == true)
                {
                    lcd.write(1);
                }
    
                
            }
        }
        else
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.print(motorSpeed[currentmotorSpeed][0]);
            if(currentmotorSpeed == 2){
                lcd.setCursor(0,0);
                lcd.print("Save Settings");
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                lcd.print("Click to Save All");
            }else{
                lcd.setCursor(0,1);
                lcd.print(parametersMotor[currentmotorSpeed]);
                lcd.setCursor(5,1);
                lcd.print(motorSpeed[currentmotorSpeed][1]);
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                if(motorEditSpeedFlag == false){
                    lcd.print("Click to Edit");   
                }else{
                    lcd.print("Click to Save");  
                }
                lcd.setCursor(19,3);
                if (fastScroll == true)
                {
                    lcd.write(1);
                }
            }
        }
    }
    else if (runAutoFlag == true)
    {
        if (grinderRunAutoFlag == true)
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Run Auto");
            lcd.setCursor(0,1);
            lcd.print("Grinder Running");
            lcd.setCursor(0,2);
            lcd.print("Time : ");
            if (minRemaining > 1)
            {
                lcd.print(minRemaining);
                lcd.print(" Minute");
            }
            else
            {
                lcd.print(secRemaining);
                lcd.print(" Seconds");
            }
            lcd.setCursor(0,3);
            lcd.print("Wait until Finish");
        }
        else if (pauseAfterGrindRunAutoFlag == true)
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Run Auto");
            lcd.setCursor(0,1);
            lcd.print("Pause After Grind");
            lcd.setCursor(0,2);
            lcd.print("Wait to Proceed or");
            lcd.setCursor(0,3);
            lcd.print("Click to Proceed");
        }
        else if (volumetricRunAutoFlag == true)
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Run Auto");
            lcd.setCursor(0,1);
            lcd.print("Volumetric Running");
            lcd.setCursor(0,2);
            lcd.print("");
            lcd.setCursor(0,3);
            lcd.print("Wait until Finish");
        }
        else if (conveyorRunAutoFlag == true)
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Run Auto");
            lcd.setCursor(0,1);
            lcd.print("Conveyor Running");
            lcd.setCursor(0,2);
            lcd.print("");
            lcd.setCursor(0,3);
            lcd.print("Wait until Finish");
        }
    }
    else if (testMenuFlag == true)
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print(TestMenuScreen[currentTestMenuScreen]);

        if (currentTestMenuScreen == 0)
        {
            if (testGrinderRunFlag == true)
            {
                lcd.setCursor(0,2);
                lcd.print("Grinder - Run");
            }
            else
            {
                lcd.setCursor(0,2);
                lcd.print("Grinder - Stop");
            }
            
            
        }
        else if (currentTestMenuScreen == 1)
        {
             if (testconveyorRunFlag == true)
            {
                lcd.setCursor(0,2);
                lcd.print("Conveyor - Run");
            }
            else
            {
                lcd.setCursor(0,2);
                lcd.print("Conveyor - Stop");
            }
        }
        else if (currentTestMenuScreen == 2)
        {   
            if (digitalRead(jar_sen) == 0)
            {
                lcd.setCursor(0,1);
                lcd.print("Jar is Detected");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("Place a Jar");
            }
            
            if (testvolumetricRunFlag == true)
            {
                lcd.setCursor(0,2);
                lcd.print("Volumetric - Run");
            }
            else
            {
                lcd.setCursor(0,2);
                lcd.print("Volumetric - Stop");
            }
        }
        
        if (currentTestMenuScreen == 3)
        {
            lcd.setCursor(0,3);
            lcd.print("Click to Exit Test");
        }else{
            lcd.setCursor(0,3);
            lcd.print("Click to Run Test");
        }
    }
    else
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0,3);
        lcd.write(0);
        lcd.setCursor(2,3);
        lcd.print(screens[currentScreen][1]);
    }
}

void save_setting(){
    grindTime =  parameters[0];
    conveyorHZ = parametersMotor[0];
    volumetricHZ =  parametersMotor[1];
    
    EEPROM.writeDouble(grindTimeAdd, grindTime);
    EEPROM.writeDouble(conveyorHZAdd, conveyorHZ);
    EEPROM.writeDouble(volumetricHZAdd, volumetricHZ);
}

void load_settings(){
    parameters[0] = EEPROM.readDouble(grindTimeAdd);
    parametersMotor[0] = EEPROM.readDouble(conveyorHZAdd);
    parametersMotor[1] = EEPROM.readDouble(volumetricHZAdd);

    grindTime = EEPROM.readDouble(grindTimeAdd);
    conveyorHZ = EEPROM.readDouble(conveyorHZAdd);
    volumetricHZ = EEPROM.readDouble(volumetricHZAdd);
}