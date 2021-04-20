#include "screen_lib.h"
#include "TFT_eSPI.h" // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include "FS.h"
#include <WiFi.h> 
#include <DNSServer.h>
#include <WebServer.h>
#include "WiFiManager.h"
#include <Arduino.h>

// // Set your Static IP address
// IPAddress local_IP(192, 168, 1, 184);
// // Set your Gateway IP address
// IPAddress gateway(192, 168, 1, 1);

// IPAddress subnet(255, 255, 0, 0);
// IPAddress primaryDNS(8, 8, 8, 8);   //optional
// IPAddress secondaryDNS(8, 8, 4, 4); //optional

void tft_setup(){
    TFT_eSPI tft = TFT_eSPI();  // Invoke library
    //scene = "MAIN";
    main_layout();
}

void main_layout()
{
  scene = "MAIN";
  tft.fillScreen(TFT_GREY);
  
  tft.setCursor(60, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE);  tft.setTextSize(2); tft.setTextFont(2);
  // We can now plot text on screen using the "print" class
  tft.println("Audio device 1");

  // // Configures static IP address
  // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
  //   Serial.println("STA Failed to configure");
  // }

  if (WiFi.status() == WL_CONNECTED)
     {
          tft.setTextColor(TFT_GREEN);  tft.setTextSize(1); tft.setTextFont(2);
          tft.print("Device verbonden met netwerk: ");
          tft.println(WiFi.SSID());
     }
  else{
          tft.setTextColor(TFT_RED);  tft.setTextSize(1); tft.setTextFont(2);
          tft.print("(Nog) niet verbonden met netwerk, setup vereist!");
  }
  //wifi setup button
  tft.fillRect(STARTCALLBUTTON_X, STARTCALLBUTTON_Y, STARTCALLBUTTON_W, STARTCALLBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WIFI setup", STARTCALLBUTTON_X + (STARTCALLBUTTON_W / 2) + 1, STARTCALLBUTTON_Y + (STARTCALLBUTTON_H / 2));
  tft.drawRect(FRAME_X, FRAME_Y, FRAME_W, FRAME_H, TFT_BLACK);

  tft.fillRect(WIFISETUPBUTTON_X, WIFISETUPBUTTON_Y, WIFISETUPBUTTON_W, WIFISETUPBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X1, FRAME_Y1, FRAME_W1, FRAME_H1, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("OPEN CALL", WIFISETUPBUTTON_X + (WIFISETUPBUTTON_W / 2) + 1, WIFISETUPBUTTON_Y + (WIFISETUPBUTTON_H / 2));
  tft.drawRect(FRAME_X1, FRAME_Y1, FRAME_W1, FRAME_H1, TFT_BLACK);
  
}

void wifi_setup_layout()
{
  scene = "WIFI_SETUP";  
  tft.fillScreen(TFT_GREY);
  
  tft.setCursor(0, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE);  tft.setTextSize(2);tft.setTextFont(2);
  // We can now plot text on screen using the "print" class
  tft.println("WIFI setup");

  WiFiManager wifiManager;
  tft.println("");
  
  tft.setTextSize(1);
  tft.println("- Start automatische wifi setup...");
  tft.println("- Bij bestaand netwerk, configuratie niet nodig!");
  tft.println("- Starting on name: Audio Conferencing Device 1");
  if(!wifiManager.autoConnect("Audio Conferencing Device 1")){
    tft.println("- Kan geen verbinding maken met WIFI!");
    tft.println("");
    tft.println("- TERUG NAAR MAIN MENU...(5s)");
    delay(5000);
    scene = "MAIN";
    main_layout();
  }
  else{
    tft.setTextColor(TFT_GREEN);
    tft.println("- Wifi is connected!");
    tft.setTextColor(TFT_WHITE);
    tft.print("- Connected to: ");
    tft.println(WiFi.SSID());
    tft.println("");
    tft.println("- TERUG NAAR MAIN MENU...(5s)");
    delay(5000);
    scene = "MAIN";
    main_layout();
    Serial.println("Wifi is connected!");
  }

  //---------------
  //  tft.print("- Wifi manager niet in deze versie toegevoegd! ");
  //  tft.println("- TERUG NAAR MAIN MENU...(5s)");
  //  delay(5000);
  //  scene = "MAIN";
  //  main_layout();
}

void open_call()
{
  scene = "OPEN_CALL";
  tft.fillScreen(TFT_GREY);
  
  tft.setCursor(60, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE);  tft.setTextSize(2); tft.setTextFont(2);
  // We can now plot text on screen using the "print" class
  tft.println("Audio device 1");


  if (WiFi.status() == WL_CONNECTED)
     {
          tft.setTextColor(TFT_GREEN);  tft.setTextSize(1); tft.setTextFont(2);
          tft.print("Device verbonden met netwerk: ");
          tft.println(WiFi.SSID());
     }
  else{
          tft.setTextColor(TFT_RED);  tft.setTextSize(1); tft.setTextFont(2);
          tft.print("(Nog) niet verbonden met netwerk, setup vereist!");
  }
  tft.setCursor(0, 200, 2);
  if(MUTE_MIC){
    tft.setTextColor(TFT_RED);  tft.setTextSize(1); tft.setTextFont(2);
    tft.print("Status mic : ");
    tft.println("MIC GEMUTE");
  }
  tft.setCursor(0, 215, 2);
  if(MUTE_AMP){
    tft.setTextColor(TFT_RED);  tft.setTextSize(1); tft.setTextFont(2);
    tft.print("Status amp : ");
    tft.println("AMP IS MUTE");
  }

  //wifi setup button
  tft.fillRect(MUTEMICBUTTON_X, MUTEMICBUTTON_Y, MUTEMICBUTTON_W, MUTEMICBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X2, FRAME_Y2, FRAME_W2, FRAME_H2, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CLOSE CALL", MUTEMICBUTTON_X + (MUTEMICBUTTON_W / 2) + 1, MUTEMICBUTTON_Y + (MUTEMICBUTTON_H / 2));
  tft.drawRect(FRAME_X2, FRAME_Y2, FRAME_W2, FRAME_H2, TFT_BLACK);

  tft.fillRect(LISTENBUTTON_X, LISTENBUTTON_Y, LISTENBUTTON_W, LISTENBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X3, FRAME_Y3, FRAME_W3, FRAME_H3, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  if(MUTE_AMP){
    tft.drawString("UNMUTE AMP", LISTENBUTTON_X + (LISTENBUTTON_W / 2) + 1, LISTENBUTTON_Y + (LISTENBUTTON_H / 2));
  }
  else{
    tft.drawString("MUTE AMP", LISTENBUTTON_X + (LISTENBUTTON_W / 2) + 1, LISTENBUTTON_Y + (LISTENBUTTON_H / 2));
  }  
  tft.drawRect(FRAME_X3, FRAME_Y3, FRAME_W3, FRAME_H3, TFT_BLACK);

    tft.fillRect(CLOSECALLBUTTON_X, CLOSECALLBUTTON_Y, CLOSECALLBUTTON_W, CLOSECALLBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X4, FRAME_Y4, FRAME_W4, FRAME_H4, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  if(MUTE_MIC){
    tft.drawString("UNMUTE MIC", CLOSECALLBUTTON_X + (CLOSECALLBUTTON_W / 2) + 1, CLOSECALLBUTTON_Y + (CLOSECALLBUTTON_H / 2));
  }
  else{
    tft.drawString("MUTE MIC", CLOSECALLBUTTON_X + (CLOSECALLBUTTON_W / 2) + 1, CLOSECALLBUTTON_Y + (CLOSECALLBUTTON_H / 2));
  }
  tft.drawRect(FRAME_X4, FRAME_Y4, FRAME_W4, FRAME_H4, TFT_BLACK);

  tft.fillRect(MUTEAMPBUTTON_X, MUTEAMPBUTTON_Y, MUTEAMPBUTTON_W, MUTEAMPBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X5, FRAME_Y5, FRAME_W5, FRAME_H5, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("LISTENER", MUTEAMPBUTTON_X + (MUTEAMPBUTTON_W / 2) + 1, MUTEAMPBUTTON_Y + (MUTEAMPBUTTON_H / 2));
  tft.drawRect(FRAME_X5, FRAME_Y5, FRAME_W5, FRAME_H5, TFT_BLACK);
  
}

void listener_layout()
{
  scene = "LISTENER_LAYOUT";  
  tft.fillScreen(TFT_GREY);
  
  tft.setCursor(60, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE);  tft.setTextSize(2); tft.setTextFont(2);
  // We can now plot text on screen using the "print" class
  tft.println("Audio device 1");


  if (WiFi.status() == WL_CONNECTED)
     {
        tft.setTextColor(TFT_GREEN);  tft.setTextSize(1); tft.setTextFont(2);
        tft.print("Device verbonden met netwerk: ");
        tft.println(WiFi.SSID());
     }
  else{
        tft.setTextColor(TFT_RED);  tft.setTextSize(1); tft.setTextFont(2);
        tft.print("(Nog) niet verbonden met netwerk, setup vereist!");
  }
  //wifi setup button
  tft.fillRect(BACKTOCALLBUTTON_X, BACKTOCALLBUTTON_Y, BACKTOCALLBUTTON_W, BACKTOCALLBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X6, FRAME_Y6, FRAME_W6, FRAME_H6, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("BACKTOCALL", BACKTOCALLBUTTON_X + (BACKTOCALLBUTTON_W / 2) + 1, BACKTOCALLBUTTON_Y + (BACKTOCALLBUTTON_H / 2));
  tft.drawRect(FRAME_X6, FRAME_Y6, FRAME_W6, FRAME_H6, TFT_BLACK);

  tft.fillRect(BLOCKLISTENERBUTTON_X, BLOCKLISTENERBUTTON_Y, BLOCKLISTENERBUTTON_W, BLOCKLISTENERBUTTON_H, TFT_WHITE);
  tft.drawRect(FRAME_X7, FRAME_Y7, FRAME_W7, FRAME_H7, TFT_BLACK);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("BLOCKLISTENER", BLOCKLISTENERBUTTON_X + (BLOCKLISTENERBUTTON_W / 2) + 1, BLOCKLISTENERBUTTON_Y + (BLOCKLISTENERBUTTON_H / 2));
  tft.drawRect(FRAME_X7, FRAME_Y7, FRAME_W7, FRAME_H7, TFT_BLACK);
  
}

// void touch_calibrate()
// {
//   uint16_t calData[5];
//   uint8_t calDataOK = 0;

//   // check file system exists
//   if (!SPIFFS.begin()) {
//     Serial.println("Formating file system");
//     SPIFFS.format();
//     SPIFFS.begin();
//   }

//   // check if calibration file exists and size is correct
//   if (SPIFFS.exists(CALIBRATION_FILE)) {
//     if (REPEAT_CAL)
//     {
//       // Delete if we want to re-calibrate
//       SPIFFS.remove(CALIBRATION_FILE);
//     }
//     else
//     {
//       File f = SPIFFS.open(CALIBRATION_FILE, "r");
//       if (f) {
//         if (f.readBytes((char *)calData, 14) == 14)
//           calDataOK = 1;
//         f.close();
//       }
//     }
//   }

//   if (calDataOK && !REPEAT_CAL) {
//     // calibration data valid
//     tft.setTouch(calData);
//   } else {
//     // data not valid so recalibrate
//     tft.fillScreen(TFT_BLACK);
//     tft.setCursor(20, 0);
//     tft.setTextFont(2);
//     tft.setTextSize(1);
//     tft.setTextColor(TFT_WHITE, TFT_BLACK);

//     tft.println("Touch corners as indicated");

//     tft.setTextFont(1);
//     tft.println();

//     if (REPEAT_CAL) {
//       tft.setTextColor(TFT_RED, TFT_BLACK);
//       tft.println("Set REPEAT_CAL to false to stop this running again!");
//     }

//     tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

//     tft.setTextColor(TFT_GREEN, TFT_BLACK);
//     tft.println("Calibration complete!");

//     // store data
//     File f = SPIFFS.open(CALIBRATION_FILE, "w");
//     if (f) {
//       f.write((const unsigned char *)calData, 14);
//       f.close();
//     }
//   }
// }