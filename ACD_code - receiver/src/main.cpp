#include <Arduino.h>
#include <WiFi.h> 
#include "esp_wpa2.h"
#include <DNSServer.h>
#include <WebServer.h>
#include "WiFiManager.h"

#include "screen_lib.h"


#include <WiFiUdp.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <math.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "driver/i2s.h"

// //error typee
// esp_err_t err;

// //buffers
// int rxbuf[256], txbuf[256];
// float l_in[128], r_in[128];
// float l_out[128], r_out[128];

// char pktbuf1[10];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

String scene;
TFT_eSPI tft;
TaskHandle_t Task1;

bool CALL_TASK;
bool MUTE_MIC;
bool MUTE_AMP;

#define PIN_MUTE_MIC 16
#define PIN_MUTE_AMP 17

//++++++++++++++++++++++++++++++++++++++++
//I2s buffers
#define SERIAL_BAUD 115200
#define SAMPLE_RATE 16000
#define MTU 1440

int16_t sample16 = 0;
int16_t sample = 0;
int16_t element = 0;
int sample_counter = 0;

//++++++++++++++++++++++++++++++++++++++++
//UDP settings
const char * udpAddress = "172.20.10.5"; //ip pc //zwart timo hotspot: 172.20.10.2
const int udpPort = 3333;
boolean transmit = false;
QueueHandle_t queueSample;
int queueSize = SAMPLE_RATE * 2;
const i2s_port_t I2S_PORT = I2S_NUM_0;
//create UDP instance
WiFiUDP udp;
//++++++++++++++++++++++++++++++++++++++++

void setup(void) {
  Serial.begin(115200);
  REG_WRITE(PIN_CTRL, 0xFF0); 
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
  CALL_TASK = false;
  MUTE_MIC = false;
  MUTE_AMP = false;

  pinMode(PIN_MUTE_MIC, OUTPUT);
  pinMode(PIN_MUTE_AMP, OUTPUT);

 Serial.println("Configuring I2S...");
  // The I2S config as per the example
  esp_err_t err;
 const  i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX  | I2S_MODE_RX), // Receive, not transfer
    .sample_rate = 16000,                         // 16KHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // could only get it to work with 32bits
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // although the SEL config should be left, it seems to transmit on right
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
    .dma_buf_count = 32,                           // number of buffers
    .dma_buf_len = 64,
    .use_apll = true
  };
  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 27,   // BCKL
    .ws_io_num = 26,    // LRCL
    .data_out_num = 25, // not used (only for speakers)
    .data_in_num = 33   // DOUT
  };
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }  

  Serial.println("I2S driver installed.");

  tft.init();
  tft.setRotation(1);
  main_layout();
}

//Sample audio task: core 0/ core 1
void sampleAudio( void * parameter )
{      
      while ( true ){  
        if(CALL_TASK == false){
          vTaskDelete(NULL);
        }    
        i2s_pop_sample(I2S_PORT, (char *)&sample,10);
        //i2s_write_bytes(I2S_PORT, (const char*)&sample, sizeof(uint32_t), 0);
        sample>>=14;
        sample16 = (int32_t)sample;
        xQueueSend(queueSample, &sample, 10);     
        //xQueueSend(queueSample, &sample, 10);    
      }
      vTaskDelete( NULL ); 
}

void receiveUDP( void * parameter )
{     
      unsigned char packetBuffer[1440]; //buffer to hold incoming packet,
      while ( true ){  
        if(CALL_TASK == false){
          vTaskDelete(NULL);
        }    
        if(udp.parsePacket());
        udp.read(packetBuffer,1024);
        size_t bytes_written;
        i2s_write((i2s_port_t)0, packetBuffer, sizeof(packetBuffer), &bytes_written, portMAX_DELAY);
        //i2s_write((i2s_port_t)0, samples_data, ((bits+8)/16)*SAMPLE_PER_CYCLE*4, &i2s_bytes_write, 100);
      }
      vTaskDelete( NULL ); 
}

//Task1code: core 0 
void Task1code( void * pvParameters ){
  // Serial.print("Thinger.io running on core ");
  // Serial.println(xPortGetCoreID());

  for(;;){
    if(CALL_TASK == false){
      vTaskDelete(NULL);
    }
    udp.beginPacket(udpAddress, udpPort);        
      for(sample_counter=0; sample_counter<MTU/2; sample_counter++){         
        xQueueReceive(queueSample, &element, portMAX_DELAY);
        // int32_t sampletx = (int32_t)element;
        // sampletx<<=14;
        //i2s_write_bytes(I2S_PORT, (const char*)&sampletx, sizeof(uint32_t), 0);
        //Serial.println(element);
        udp.write((byte*)&element,2);     
      }        
    udp.endPacket(); 
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
          queueSample = xQueueCreate( queueSize, sizeof( int16_t ) );   
            if(queueSample == NULL){      
          Serial.println("Error creating the queue");   
          }   
          udp.begin(3333);
          xTaskCreate(
                    receiveUDP,          /* Task function. */
                    "receiveUDP",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

          // xTaskCreate(
          //           sampleAudio,          /* Task function. */
          //           "SampleAudio",        /* String with name of task. */
          //           10000,            /* Stack size in words. */
          //           NULL,             /* Parameter passed as input of the task */
          //           1,                /* Priority of the task. */
          //           NULL);            /* Task handle. */

          // xTaskCreatePinnedToCore(
          //           Task1code,   /* Task function. */
          //           "Task1",     /* name of task. */
          //           10000,       /* Stack size of task */
          //           NULL,        /* parameter of the task */
          //           1,           /* priority of the task */
          //           &Task1,      /* Task handle to keep track of created task */
          //           0);          /* pin task to core 0 */                  
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