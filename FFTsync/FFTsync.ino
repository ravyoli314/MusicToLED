#include <arduinoFFT.h>

#define sensorPin A0
#define REDPIN 5
#define GREENPIN 3
#define BLUEPIN 6

#define SAMPLES 64             //Must be a power of 2
#define SAMPLING_FREQUENCY 1000 //Hz, must be less than 10000 due to ADC

#define Rrange 10 // SAMPLES/3 (more or less) we'll devide de frequency array into 3 ranges
#define Grange 11
#define Brange 10 
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds; // To store the current time since the Arduino board started 
 
double vReal[SAMPLES];
double vImag[SAMPLES];

double Rmodule = 0;
double Gmodule = 0;
double Bmodule = 0;

int Rvalue = 0;
int Gvalue = 0;
int Bvalue = 0;

void setup() {
  Serial.begin(115200); // open the serial port at 115200 bps:
  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
  
  pinMode(sensorPin, INPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
}

void loop() {

  /*SAMPLING*/
  
  for(int i=0; i<SAMPLES; i++)
  {
    microseconds = micros();    //Overflows after around 70 minutes
    vReal[i] = analogRead(sensorPin);
    vImag[i] = 0;
    
    while(micros() < (microseconds + sampling_period_us)){ // We take sampling_period_us microseconds to read the next sample
      }
   }
   /*FFT*/
   
   FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
   FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
   FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
   
   Rvalue = 0;
   Gvalue = 0;
   Bvalue = 0;

   Rmodule = 0;
   Gmodule = 0;
   Bmodule = 0;

   /* MODULES of the frequency vectors computed */
   
   for(int i=1; i < SAMPLES/2; i++)
   {
    double a = pow(vReal[i], 2);
    double b = pow(vImag[i], 2);

    // Now we assign each frequency value to its corresponding light range:
    if(i <= Rrange) Rmodule += sqrt(a + b);
    else if(i >= (Rrange+1) && i <= (Rrange+Grange)) Gmodule += sqrt(a + b);
    else Bmodule += sqrt(a + b);
  }

  // Mean values of the modules of each interval:
  Rmodule /= Rrange; 
  Gmodule /= Grange;
  Bmodule /= Brange;

  /* Giving more INTENSITY to the most DOMINANT frequencies (and reducing the rest):*/
  
  if((Rmodule > Gmodule) && (Gmodule > Bmodule)){
    Rmodule *= 1.5; 
    Gmodule *= 0.5;
    Bmodule *= 0.5;
    }
   else if((Bmodule > Rmodule)&& (Bmodule > Gmodule)){
    Bmodule *= 1.5;
    Gmodule *= 0.2;
    Rmodule *= 0.2;
    }
   else if((Gmodule > Bmodule) && (Gmodule > Rmodule)){
    Gmodule *= 1.5;
    Rmodule *= 0.2;
    Bmodule *= 0.2;
    }

  Rvalue = 255 * convBrightness(Rmodule);
  Gvalue = 255 * convBrightness(Gmodule);
  Bvalue = 255 * convBrightness(Bmodule);

/*
  Serial.println(Rvalue);
  Serial.println(Gvalue);
  Serial.println(Bvalue);
 */

  analogWrite(REDPIN, Rvalue);
  analogWrite(GREENPIN, Gvalue);
  analogWrite(BLUEPIN, Bvalue);

  delay(25);

}

double convBrightness(double b) {
  double c = b / 614; // The maximun intensity value in theory is 31713 (but we are never having the volume that high)
  if( c < 0.2 ) {
    c = 0;
  }
  else if(c > 1) {
    c = 1.00;
  }
  return c;
  }
