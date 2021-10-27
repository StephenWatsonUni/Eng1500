#include <Wire.h> //I2C Communication Protcol.
#include <Adafruit_GFX.h> //graphics core.
#include <Adafruit_SSD1306.h> //display driver.
#define Threshold 500
#define UpperThreshold 525
#define LowerThreshold 475
//(next two lines) used to define the display size in pixels.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins).
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // '-1' makes display reset = on-board reset pin.
int ECG = A0;
int sample;
unsigned long time1;
unsigned long time2;
unsigned long peak;
unsigned long lastpeak;
unsigned long period;
unsigned long BPM;
unsigned long AvBPM;
unsigned long BPMm2;
unsigned long BPMm1;
unsigned long MaxBPM = 0;
unsigned long MinBPM = 300;
//unsigned long bpmAverages[100];
//unsigned long totalAverage;
//int n = 0;
int state = 1;

void setup() { 
 Serial.begin (9600); //using 9600 for communication via USB A to USB B
 pinMode(ECG, INPUT); //establishes 'ECG' as a device that will send information to the Arduino.
 //Intiation sequence for OLED-screen. Only needs to be ran once (per start-up) and is therefor not included in a loop.
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //'0x3C' is the I2C address for the Arduino. It can be found by running a line of code.
 display.display(); //When a 'display.(setting)' function is run, it stores the function and its information to a cache, where it can be called upon with 'display.display' to make the display run and display the series of code stored.
 delay(2000); //delay is needed to allow the display to boot up, otherwise problems occur like text overlapping which can lead to the arduino boot looping due to memory leak.
 display.clearDisplay();
 display.display();
 display.clearDisplay();//Clears the current cache for the display
 //Displays team name. Allows user to associate product with a name.
 display.setTextSize(1);
 display.setTextColor(WHITE); //In this case, the OLED screen can only display white pixels.
 display.setCursor(32, 18); //Sets the point on the screen where the first pixel will display.
 display.print("Cardia");
 display.println("    Tech"); //(currently) the space presnt before "Tech" is to centre the word directly below the previous.
  delay(4000);
 display.clearDisplay();
 //Clears buffer, and thus what is displayed on-screen
 
}


void loop() {
//Start of heart rate detection implementing our chosen threshold values
  while(state == 1){ //state 1: code reads ECG value and determines whether or not it is below or above an ECG reading of 500
    while(analogRead(ECG) < Threshold){ //if it is below 500, the arduino will wait 10ms, and then read the ECG value again.
      delay(10);
    }
  state = 2; //if the ECG value is not below the threshold and instead is either equal to or above. The while loop will break and move on to state = 2.
  }
  
  while(state == 2){ //state 2: as the ECG value becomes => than the threshold.
    time1 = millis(); //the time (in ms) from state 1 to state 2 is stored as time1. This is the ms it took for the ECG to rise above the threshold.
   state = 3;
  }
  
  while(state == 3){ //state 3: at this point, the QRS wave would of passed the threshold, and rise until it crosses the upper threshold.
    while(analogRead(ECG) < UpperThreshold){ //waits until the upper threshold is met or passed
      delay(10);
    }
    state = 4;
  }
  
  while(state == 4){ //state 4: the peak of the R wave should be the only thing consisiting above the upper threshold.
    while(analogRead(ECG) > Threshold){ //waits for the R wave to come back down to the Threshold.
      delay(10);
    }
    state = 5;
  }
  
  while(state == 5){
    //n++;
    time2 = millis(); //time between state 5 and state 1 is stored as time2. This is the ms it took for the ECG to fall below the threshold.
    peak = (time1+time2)/2; //anything above the threshold will be the peak of the QRS wave, and to get the the exact point of the time of the peak, an average between time1 and time2 can be calculated.
    period = peak-lastpeak; 
    BPM = 60000/period; //converts ECG value to BPM (60000/period = 60s * 1000ms * 1/period)
    AvBPM = ((BPMm2+BPMm1+BPM)/3); //filtering the BPM by using an average of 3 consecutive heart rates.
    if(AvBPM > MaxBPM){ //compares average BPM to the highest BPM recorded.
      MaxBPM = AvBPM; //if higher, the current average BPM becomes the new highest BPM. 
    }
    else if(AvBPM < MinBPM){ //if instead, the average BPM is lower than the lowest recorded BPM.
      MinBPM = AvBPM; //the average BPM becomes the new lowest BPM record.
    }
   /* for(int i = (n - 1); i < n; i++){
      if(i > 99){
        
      }
      bpmAverages[i] = AvBPM;
      totalAverage = (bpmAverages[i] / n);
    }*/
    Serial.println(analogRead(A0));
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);//(3, 1);
    display.println("Heart Rate:");
    display.setCursor(0, 8);//(35, 32);
    display.setTextSize(2);
    display.write(3);
    display.setCursor(16, 8);
    display.print(AvBPM);
    display.print(" BPM");
    display.setCursor(24, 32);
    display.setTextSize(1);
    display.print("You are dying!");
    display.setTextSize(1);
    display.setCursor(0, 48);
    display.print("Max: ");
    display.print(MaxBPM);
    display.setCursor(0, 56);
    display.print("Min: ");
    display.print(MinBPM);
    //display.startscrollleft(0x07, 0x07);
    //Total average will be included when SD card is implemented. 
    /*display.print(" Average Heart Rate: ");
    display.print(totalAverage);*/   
    display.display();
    BPMm2 = BPMm1; //The BPM value that was read previous to the newest BPM value becomes the oldest read BPM.
    BPMm1 = BPM; //The most recent read BPM becomes the BPM that was read before the next reading of the BPM.
    lastpeak = peak; //current time of the R peak becomes the previous peak time.
    
    state = 6;
  }

  while(state == 6){ //state 6: checks if the ECGs value has fallen below the average threshold.
    while(analogRead(ECG)>LowerThreshold){ //If it has, then the loop will break and state will return to 1. This makes sure the heart wave has reset.
      delay(10);
    }
    state=1;
  }
 


}
