//
// System library
//
#include <MsTimer2.h>
#include <SoftwareSerial.h>

typedef struct
{
  int led;
  long previousMillis;
  boolean currentState;
  boolean state;
  int timer;
} light;

//
// Program variables
// LEFT:    13
// RIGHT:   12
// LIGHT1:  11
// CRANE:   10
//
int lightningLeds[]= {13, 12, 11, 10};
light lights[sizeof(lightningLeds)];

//
// Lightning control
//
void setLightningControl(int led, boolean state, int timer = 0)
{
  if (state == 0 && timer == 0)
    digitalWrite(led, LOW);
  else if (state == 1 && timer == 0)
    digitalWrite(led, HIGH);
  else if (timer > 0 && timer < 10000) 
    digitalWrite(led, state);
}

//
// Timer function
//
void flash() 
{
  for (int ii=0; ii<sizeof(lightningLeds); ii++)
  {
    if (lights[ii].state == 1 && lights[ii].timer > 0 )
    {
      //
      // Estimate the delay using millis
      //
      unsigned long currentMillis = millis();
      if (currentMillis - lights[ii].previousMillis > (long)lights[ii].timer)
      {
        lights[ii].previousMillis = currentMillis;
        lights[ii].currentState = !lights[ii].currentState;
        setLightningControl(lights[ii].led, lights[ii].currentState, lights[ii].timer);
      }
    }

    //
    // Compare if the state and the current state are different
    //
    if (lights[ii].currentState != lights[ii].state && lights[ii].timer == 0)
    {
      lights[ii].previousMillis = 0;
      lights[ii].currentState = lights[ii].state;
      setLightningControl(lights[ii].led, lights[ii].state, lights[ii].timer);
    }
    
  }
}

//
// Setup routine
//
void setup()
{
  for (int ii=0; ii<sizeof(lightningLeds); ii++)
  {
    lights[ii] = (light) {lightningLeds[ii], 0, 0, 0, 0};
    pinMode(lights[ii].led, OUTPUT);
  }

  MsTimer2::set(50, flash); 
  MsTimer2::start();

  // 
  // Start the lightning mode using bluetooth
  //
  Serial1.begin(9600);
}

//
// Loop function
//
String readString = "";
char readChar;
int serialLed = 0;
int serialState = 0;
int serialTimer = 0;
int readIndex = 0;
void loop()
{
  if (Serial1.available())
  {
    //
    // Read the character
    //
    readChar = Serial1.read();
    if (isDigit(readChar))
      readString += readChar;

    //
    // Search parameters
    //
    if (readChar == ',') 
    {
      if (readIndex == 0) 
        serialLed = readString.toInt();
      else if (readIndex == 1) 
        serialState = readString.toInt();
      
      readString = "";
      readIndex++;
    }
  }

  if (readChar == ';') 
  {
    if (readIndex == 2) serialTimer = readString.toInt();
    
    Serial1.println("serialLed: " + String(serialLed));
    Serial1.println("serialState: " + String(serialState));
    Serial1.println("serialTimer: " + String(serialTimer));
    readString = "";
    readChar = "";
    readIndex = 0;

    // 
    // Lightning states
    //
    if (serialLed >= 0 && serialLed <= sizeof(lightningLeds))
    {
      //
      // If the state if OFF stop the timer
      //
      lights[serialLed].state = serialState; 
      if (serialState == 0)
        lights[serialLed].timer = 0; 
      
      if (serialState == 1 && serialTimer >= 0)
        lights[serialLed].timer = serialTimer; 
    }
    else
    {
      for (int ii=0; ii<sizeof(lightningLeds); ii++)
      {
        lights[ii].state = 0;
        lights[ii].timer = 0;
      }
    }
  }
}
