#include <Water_Wastage_Prevention_inferencing.h>
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "thingProperties.h"

#include "HUSKYLENS.h"

HUSKYLENS huskylens;

Audio audio;

#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

#define SENSOR 14

volatile int  Pulse_Count;
unsigned int  flowrate;
unsigned long Current_Time, Loop_Time;

byte ei_flag = 0;
byte audio_flag = 0;

unsigned long detectionStartTime = 0;
unsigned long detectionDuration = 0;

float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) 
{
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}


void setup()
{
    Serial.begin(115200);
    Wire.begin();
    Serial.println("Edge Impulse Inferencing Demo");
    pinMode(SENSOR, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(SENSOR), Detect_Rising_Edge, RISING);
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin("DARKMATTER", "8848191317");

    while (WiFi.status() != WL_CONNECTED)
    delay(1500);

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(100);
    initProperties();
    ArduinoCloud.begin(ArduinoIoTPreferredConnection);
    setDebugMessageLevel(2);
    ArduinoCloud.printDebugInfo();

    while (!huskylens.begin(Wire))
    {
        Serial.println(F("Begin failed!"));
        delay(100);
    }
    
}

void updateFeatures() 
{
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i += 2) {
     Current_Time = millis();
     if(Current_Time >= (Loop_Time + 500))
     {
        Loop_Time = Current_Time;
        flowrate = (Pulse_Count * 60 / 7.5);
        flowRate = (Pulse_Count/7.5);
        Pulse_Count = 0;
     }  
     features[i + 0] = flowrate;
     features[i + 1] = flowrate;
     delay(EI_CLASSIFIER_INTERVAL_MS);
    }
}

void Detect_Rising_Edge ()
{ 
   Pulse_Count++;
} 


void loop()
{
    if(ei_flag == 0)
    {
      ArduinoCloud.update();
      run_inference();
    }
    if(audio_flag == 1)
    {
      audio.connecttospeech("Please turn off the pipe and save the water for the future", "en");
      audio_flag = 0;
    }
     audio.loop();
    
}

void audio_info(const char* info)
{
  Serial.print("audio_info: ");
  Serial.println(info);

  if (strcmp(info, "End of speech: \"Please turn off the pipe and save the water for the future\"") == 0)
  {
    ei_flag = 0;
  }
}

void run_inference()
{
  ei_printf("Edge Impulse standalone inferencing (Arduino)\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        delay(1000);
        return;
    }

    ei_impulse_result_t result = { 0 };

    updateFeatures();

    signal_t features_signal;
    features_signal.total_length = sizeof(features) / sizeof(features[0]);
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    ei_printf("run_classifier returned: %d\n", res);

    if (res != 0) return;

    // print the predictions
    /*ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    ei_printf("[");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("%.5f", result.classification[ix].value);
        #if EI_CLASSIFIER_HAS_ANOMALY == 1
                ei_printf(", ");
        #else if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
                    ei_printf(", ");
                }
        #endif
    }
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
        ei_printf("%.3f", result.anomaly);
    #endif
        ei_printf("]\n");
    */

    // human-readable predictions
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) 
    {
        //ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        if (result.classification[ix].value > 0.7)
        {
          Serial.println(result.classification[ix].label);
          if (strcmp(result.classification[ix].label, "leak") == 0 || strcmp(result.classification[ix].label, "no_flow") == 0) 
          {
                if (detectionStartTime == 0) 
                {
                    // Start of detection
                    detectionStartTime = millis();
                } 
                else 
                {
                    // Detection is already in progress, update the duration
                    detectionDuration = millis() - detectionStartTime;
                }
          }
          else
          {
            detectionStartTime = 0;
          }

        }
    }

    if (detectionDuration > 60000 && ei_flag == 0) 
    {   huskylens.request();
        HUSKYLENSResult result = huskylens.read();
        Serial.print("Face ID :");
        Serial.print(result.ID);
        Serial.println();
        if (result.ID ==1)
        {
        Serial.println("Audio");
        ei_flag = 1;
        audio_flag = 1;
        detectionStartTime = 0;
        detectionDuration = 0;
        }
        else
        {
        Serial.println("IOT");
        detectionStartTime = 0;
        detectionDuration = 0;
        wastage = "True";
        }
    }
}

void onWastageChange()  {
}
