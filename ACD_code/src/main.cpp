//Project Audio Conferencing Device 
//https://github.com/Nielsvanhagen/audio-conferencing-device
//Hogeschool Rotterdam 
//Made by: Niels van Hagen, Yari de Boed, Jonatan Vrijenhoek and Timo de Vries


#include <Arduino.h>

//Network libraries
#include <WiFi.h> 
#include <WiFiUdp.h>
#include "esp_wpa2.h"
#include <DNSServer.h>
#include <WebServer.h>
#include "WiFiManager.h"

//Handmade lib for screen layout
#include "screen_lib.h" 

//Standard libraries
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <math.h>

//audio library
#include "driver/i2s.h"

//IMPORTANT VARIABLES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char * udpAddress = "x.x.x.x"; //Target ip, ip of other device 
bool LISTEN = true; //Boolean switch function of device (sender of receiver)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Local var
String scene; //Scene selection
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

//Sample variables
int16_t sample16 = 0;
int32_t sample = 0;
int16_t element = 0;
int sample_counter = 0;
esp_err_t err;

//++++++++++++++++++++++++++++++++++++++++
//UDP settings
const int udpPort = 3333;
boolean transmit = false;
QueueHandle_t queueSample;
int queueSize = SAMPLE_RATE * 2;
const i2s_port_t I2S_PORT = I2S_NUM_0;
//create UDP instance
WiFiUDP udp;
//++++++++++++++++++++++++++++++++++++++++


//------------------------------------------
//i2s settings
const  i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX), // Receive, not transfer
    .sample_rate = 16000,                         // 16KHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // could only get it to work with 32bits
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // although the SEL config should be left, it seems to transmit on right
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
    .dma_buf_count = 32,                           // number of buffers
    .dma_buf_len = 64,
    .use_apll = true
  };

  //sender config
  const  i2s_config_t i2s_config_sender = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), // Receive, not transfer
    .sample_rate = 16000,                         // 16KHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
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
    .data_out_num = 25, // DIN
    .data_in_num = 33   // DOUT
  };

  //------------------------------------------


void setup(void) {
  Serial.begin(115200);
  REG_WRITE(PIN_CTRL, 0xFF0); 
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);

  //Boolean audio 
  CALL_TASK = false;//call on/off
  MUTE_MIC = false; //mute mic
  MUTE_AMP = false; //mute amp

  //set pinmodes mute 
  pinMode(PIN_MUTE_MIC, OUTPUT);
  pinMode(PIN_MUTE_AMP, OUTPUT);

 Serial.println("Configuring I2S...");

  //Set i2s settings
  if(LISTEN){
    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
      Serial.printf("Failed installing driver: %d\n", err);
      while (true);
    }
  }
  else{
    err = i2s_driver_install(I2S_PORT, &i2s_config_sender, 0, NULL);
    if (err != ESP_OK) {
      Serial.printf("Failed installing driver: %d\n", err);
      while (true);
    }
  }  
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }  

  Serial.println("I2S driver installed.");

  //initialize display
  tft.init();
  tft.setRotation(1);
  main_layout();
}

//Receive audio from udp protocol
void receiveUDP( void * parameter )
{     
      unsigned char packetBuffer[1440]; //buffer to hold incoming packet,
      while (CALL_TASK){  
        //stop task
        if(CALL_TASK == false){
          return;
          vTaskDelete(NULL);
        }    
        //read audio buffer from udp
        udp.parsePacket();
        udp.read(packetBuffer,1024);
        //Write audio buffer to amplifier
        size_t bytes_written;
        i2s_write((i2s_port_t)0, packetBuffer, sizeof(packetBuffer), &bytes_written, portMAX_DELAY);
      }
      vTaskDelete( NULL ); 
}

//Sample audio task: core 0/ core 1
void sampleAudio( void * parameter )
{      
      while ( true ){  
        //stop task
        if(CALL_TASK == false){
          vTaskDelete(NULL);
        }    
        //read samples from i2s mic
        i2s_pop_sample(I2S_PORT, (char *)&sample,10);
        //bitshift samples
        sample>>=14;
        sample16 = (int16_t)sample;
        //send audio samples to queue
        xQueueSend(queueSample, &sample16, 10);         
      }
      vTaskDelete( NULL ); 
}

//Task1code: core 0 : sending audio samples to udp 
void Task1code( void * pvParameters ){

  for(;;){
    //stop task
    if(CALL_TASK == false){
      vTaskDelete(NULL);
    }
    //begin udp packet
    udp.beginPacket(udpAddress, udpPort);        
      for(sample_counter=0; sample_counter<MTU/2; sample_counter++){         
        xQueueReceive(queueSample, &element, portMAX_DELAY);
        // int32_t sampletx = (int32_t)element;
        // sampletx<<=14;
        //i2s_write_bytes(I2S_PORT, (const char*)&sampletx, sizeof(uint32_t), 0);
        //Serial.println(element);
        //write sample to udp
        udp.write((byte*)&element,2);     
      }         
    udp.endPacket(); 
  }
}

//open audio call 
void open_audio_call(){
  //set audio booleans
  MUTE_MIC = false;
  CALL_TASK = true;

  queueSample = xQueueCreate( queueSize, sizeof( int16_t ) );   
  if(queueSample == NULL){      
    Serial.println("Error creating the queue");   
  }   

  //listen or talk
  if(LISTEN){
    Serial.println("Device listenening...");

    udp.begin(3333);
    //mic on amp off
    MUTE_MIC = true;
    MUTE_AMP = false;
    pinMode(PIN_MUTE_MIC, HIGH); //set mic mute
    pinMode(PIN_MUTE_AMP, LOW);  //set amp mute

    xTaskCreate(
              receiveUDP,          /* Task function. */
              "receiveUDP",        /* String with name of task. */
              10000,            /* Stack size in words. */
              NULL,             /* Parameter passed as input of the task */
              1,                /* Priority of the task. */
              NULL);            /* Task handle. */
  }
  else{
    Serial.println("Device is talking...");

    //mic false amp on
    MUTE_MIC = false;
    MUTE_AMP = true;
    pinMode(PIN_MUTE_MIC, LOW);
    pinMode(PIN_MUTE_AMP, HIGH);

    //create task audio sample
    xTaskCreate(
                    sampleAudio,          /* Task function. */
                    "SampleAudio",        /* String with name of task. */
                    10000,            /* Stack size in words. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

    //network task
    xTaskCreatePinnedToCore(
              Task1code,   /* Task function. */
              "Task1",     /* name of task. */
              10000,       /* Stack size of task */
              NULL,        /* parameter of the task */
              1,           /* priority of the task */
              &Task1,      /* Task handle to keep track of created task */
              0);          /* pin task to core 0 */            
  }     
}

//loop function
void loop() {
  uint16_t x, y;

  // See if there's any touch data for us
  if (tft.getTouch(&x, &y))
  {
    //start call button
    if (scene == "MAIN"){
      if ((x > STARTCALLBUTTON_X) && (x < (STARTCALLBUTTON_X + STARTCALLBUTTON_W))) {
        if ((y > STARTCALLBUTTON_Y) && (y <= (STARTCALLBUTTON_Y + STARTCALLBUTTON_H))) {
          if (WiFi.status() == WL_CONNECTED)
          {
          open_audio_call();
          open_call();
          }
          delay(500); 
        }
      }
      //wifi setup button 
      if ((x > WIFISETUPBUTTON_X) && (x < (WIFISETUPBUTTON_X + WIFISETUPBUTTON_W))) {
        if ((y > WIFISETUPBUTTON_Y) && (y <= (WIFISETUPBUTTON_Y + WIFISETUPBUTTON_H))) {
          wifi_setup_layout();
        }
      }
    }  
    if (scene == "OPEN_CALL"){
      //mute mic button
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
    //listener layout NOT USED
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