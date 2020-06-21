
#include "processImage.h"
void setup()
{
  /* Set baud rate for serial communication */
  Serial.begin(115200);
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");
  
  initEthernet();
 } 
void loop()
{
  // put your main code here, to run repeatedly:
  byte inChar;
  inChar = Serial.read();
  myusb.Task();
  if ((inChar >= '0') && (inChar <= '9'))
  {
    fileName = "source.bmp";
    doFTP(fileName);
    if (ftpStatus) // step 1
      {
        Serial.println(F("FTP OK"));
      }
      else
      {
        Serial.println(F("FTP FAIL"));
      }
  }
}
