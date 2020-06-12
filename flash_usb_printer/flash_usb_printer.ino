
#include "processImage.h"
void setup()
{
  /* Set baud rate for serial communication */
  Serial.begin(115200);
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
    for (byte i = 0; i < 10; i++)
    {
      Serial.print(" moi vua nhap vao: ");
      Serial.println(inChar);
      fileName = String(i) + ".bmp";
      Serial.print("fileName: ");
      Serial.println(fileName);
      Serial.println(F(">>>>>>>>>>>>>READ FTP >>>>>>>>>>>>"));
      if (doFTP(fileName)) // step 1
      {
        Serial.println(F("FTP OK"));
      }
      else
      {
        Serial.println(F("FTP FAIL"));
      }
    }
  }
}
