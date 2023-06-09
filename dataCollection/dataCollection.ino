
// Used to remember what mode we're in
#define NUM_MODES           2
#define MODE_IDLE           0
#define MODE_RECORDING      1

#define SAMPLE_DELAY        500 

const int Output_Pin = T0;
volatile int  Pulse_Count;
unsigned int  Liter_per_hour;
unsigned long Current_Time, Loop_Time,Start_Time, timestamp;
static uint8_t mode = MODE_IDLE;

void setup() {
  pinMode(Output_Pin, INPUT);
  
  // Start serial
  Serial.begin(115200);

  attachInterrupt(0, Detect_Rising_Edge, RISING);
  
}

void loop() {
  
  float flowrate_v;

  static unsigned long sample_timestamp = millis();
  static unsigned long start_timestamp = millis();
  if (Serial.read() == 's') {
    mode = (mode + 1);
    if (mode >= NUM_MODES) {
        mode = MODE_IDLE;
      }
      switch (mode) {
          case MODE_IDLE:
            Serial.println();
            break;
          case MODE_RECORDING:
            Serial.println("timestamp,flowrate");
            start_timestamp = millis();
            break;
          default:
            break;
        }
      }

  // Only collect if not in idle
  if (mode > MODE_IDLE) {
    if ((millis() - sample_timestamp) >= SAMPLE_DELAY) {
      sample_timestamp = millis();
      Current_Time = millis();
     if(Current_Time >= (Loop_Time + 500))
     {
        Loop_Time = Current_Time;
        Liter_per_hour = (Pulse_Count * 60 / 7.5);
        Pulse_Count = 0;
        Serial.print(sample_timestamp - start_timestamp);
        Serial.print(",");
        Serial.print(Liter_per_hour,DEC);
        Serial.println();
     }
    }
  }
}

void Detect_Rising_Edge ()
{ 
   Pulse_Count++;
} 