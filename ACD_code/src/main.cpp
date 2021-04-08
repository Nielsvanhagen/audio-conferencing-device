#include <Arduino.h>
#include <WiFi.h> 
#include <DNSServer.h>
#include <WebServer.h>
#include "WiFiManager.h"

#include "screen_lib.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "driver/i2s.h"

//error typee
esp_err_t err;

//buffers
int rxbuf[256], txbuf[256];
float l_in[128], r_in[128];
float l_out[128], r_out[128];

char pktbuf1[10];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

String scene;
TFT_eSPI tft;
TaskHandle_t Task1;

bool CALL_TASK;
bool MUTE_MIC;
bool MUTE_AMP;

#define PIN_MUTE_MIC 16
#define PIN_MUTE_AMP 17

void setup(void) {
  Serial.begin(115200);
  REG_WRITE(PIN_CTRL, 0xFF0); 
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
  CALL_TASK = false;
  MUTE_MIC = false;
  MUTE_AMP = false;

  pinMode(PIN_MUTE_MIC, OUTPUT);
  pinMode(PIN_MUTE_AMP, OUTPUT);

  i2s_config_t i2s_config = {

        .mode                 = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX  | I2S_MODE_RX), // i2s_mode_t             work mode
        .sample_rate          = 44100,                                                    // int                    sample rate
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,                                // i2s_bits_per_sample_t  bits per sample
        .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,                                     // i2s_channel_fmt_t      channel format
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,                                  // i2s_comm_format_t      communication format
        .intr_alloc_flags     = 0,                                                        // int                    Flags used to allocate the interrupt. One or multiple (ORred) ESP_INTR_FLAG_* values. See esp_intr_alloc.h for more info
        .dma_buf_count        = 6,                                                        // int                    DMA Buffer Count
        .dma_buf_len          = 512,                                                      // int                    DMA Buffer Length
        .use_apll             = true,                                                     // bool                   using APLL as main clock, enable it to get accurate clock
        .tx_desc_auto_clear   = true,                                                     // bool                   auto clear tx descriptor if there is underflow condition (helps in avoiding noise in case of data unavailability)
        .fixed_mclk           = true                                                      // int                    using fixed MCLK output. If use_apll = true and fixed_mclk > 0, then the clock output for is fixed and equal to the fixed_mclk value
                                             
    };

    err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
       Serial.print("Failed installing driver: ");
       Serial.print(err);
       while (true);
     }
//------------------------------------------------------------

    i2s_pin_config_t pin_config = {
        .bck_io_num   = 27,
        .ws_io_num    = 26,
        .data_out_num = 25,
        .data_in_num  = 33                                                       
    };

   err = i2s_set_pin(I2S_NUM_0, &pin_config);
   if (err != ESP_OK) {
      Serial.print("Failed setting pin: ");
      Serial.print(err);
      while (true);
   }

   Serial.println("I2S driver installed.");

  tft.init();
  tft.setRotation(1);
  main_layout();
}

//Task1code: runs thinger.io loop, 1 core dedicated to communications
void Task1code( void * pvParameters ){
  Serial.print("Thinger.io running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    if(CALL_TASK == false){
      vTaskDelete(NULL);
    }
    size_t readsize = 0;
    //read 256 samples (128 stereo samples)
    esp_err_t rxfb = i2s_read(I2S_NUM_0,&rxbuf[0],256*4, &readsize, 1000);
    if (rxfb == ESP_OK && readsize==256*4) {
    
      //extract stereo samples to mono buffers
      int y=0;
      for (int i=0; i<256; i=i+2) {
        l_in[y] = (float) rxbuf[i];
        r_in[y] = (float) rxbuf[i+1];
        y++;
      }
      
      
      //do something with your left + right channel samples here in the buffers l_in/r_in and ouput result to l_out and r_out (e.g. build mono sum and apply -6dB gain (*0.5)
      
      for (int i=0; i<128; i++) {
      
        l_out[i] = 0.5f * (l_in[i] + r_in[i]);
        r_out[i] = l_out[i];
        
      
      }
      
      
      //merge two l and r buffers into a mixed buffer and write back to HW
      y=0;
      for (int i=0;i<128;i++) {
      txbuf[y] = (int) l_out[i];
      txbuf[y+1] = (int) r_out[i];
      y=y+2;
      }
    
      i2s_write(I2S_NUM_0, &txbuf[0],256*4, &readsize, 1000);
    } 
  }
}

void loop() {
  uint16_t x, y;

  // See if there's any touch data for us
  if (tft.getTouch(&x, &y))
  {
    if (scene == "MAIN"){
      if ((x > STARTCALLBUTTON_X) && (x < (STARTCALLBUTTON_X + STARTCALLBUTTON_W))) {
        if ((y > STARTCALLBUTTON_Y) && (y <= (STARTCALLBUTTON_Y + STARTCALLBUTTON_H))) {
          open_call();
          MUTE_MIC = false;
          CALL_TASK = true;
          xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
          delay(500); 
        }
      }
      if ((x > WIFISETUPBUTTON_X) && (x < (WIFISETUPBUTTON_X + WIFISETUPBUTTON_W))) {
        if ((y > WIFISETUPBUTTON_Y) && (y <= (WIFISETUPBUTTON_Y + WIFISETUPBUTTON_H))) {
          wifi_setup_layout();
        }
      }
    }  
    if (scene == "OPEN_CALL"){
      if ((x > MUTEMICBUTTON_X) && (x < (MUTEMICBUTTON_X + MUTEMICBUTTON_W))) {
        if ((y > MUTEMICBUTTON_Y) && (y <= (MUTEMICBUTTON_Y + MUTEMICBUTTON_H))) {
          if(MUTE_MIC){
            MUTE_MIC = false;
            digitalWrite(PIN_MUTE_MIC, LOW);
          }
          else{
            MUTE_MIC = true;
            digitalWrite(PIN_MUTE_MIC, HIGH);
          }
          open_call(); //reopen call layout
          delay(200);
        }
      }
      if ((x > LISTENBUTTON_X) && (x < (LISTENBUTTON_X + LISTENBUTTON_W))) {
        if ((y > LISTENBUTTON_Y) && (y <= (LISTENBUTTON_Y + LISTENBUTTON_H))) {
          //listener_layout();
        }
      }
      if ((x > CLOSECALLBUTTON_X) && (x < (CLOSECALLBUTTON_X + CLOSECALLBUTTON_W))) {
        if ((y > CLOSECALLBUTTON_Y) && (y <= (CLOSECALLBUTTON_Y + CLOSECALLBUTTON_H))) {
          Serial.println("CLOSE CALL");
          CALL_TASK = false;
          main_layout();
        }
      }
      if ((x > MUTEAMPBUTTON_X) && (x < (MUTEAMPBUTTON_X + MUTEAMPBUTTON_W))) {
        if ((y > MUTEAMPBUTTON_Y) && (y <= (MUTEAMPBUTTON_Y + MUTEAMPBUTTON_H))) {
          if(MUTE_AMP){
            MUTE_AMP = false;
            digitalWrite(PIN_MUTE_AMP, LOW);
          }
          else{
            MUTE_AMP = true;
            digitalWrite(PIN_MUTE_AMP, HIGH);
          }
          open_call(); //reopen call layout
          delay(200);
        }
      }
    }  
    if (scene == "LISTENER_LAYOUT"){
      if ((x > BACKTOCALLBUTTON_X) && (x < (BACKTOCALLBUTTON_X + BACKTOCALLBUTTON_W))) {
        if ((y > BACKTOCALLBUTTON_Y) && (y <= (BACKTOCALLBUTTON_Y + BACKTOCALLBUTTON_H))) {
          //main_layout();
        }
      }
      if ((x > BLOCKLISTENERBUTTON_X) && (x < (BLOCKLISTENERBUTTON_X + BLOCKLISTENERBUTTON_W))) {
        if ((y > BLOCKLISTENERBUTTON_Y) && (y <= (BLOCKLISTENERBUTTON_Y + BLOCKLISTENERBUTTON_H))) {
          //main_layout();
        }
      }
    }
  }  
}