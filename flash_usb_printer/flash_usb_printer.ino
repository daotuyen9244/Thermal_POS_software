
#include "processImage.h"
uint8_t ucBuf[(HEIGHT / 8) * WIDTH];
void setup()
{
  /* Set baud rate for serial communication */
  Serial.begin(115200);
  delay(100);
#ifdef USE_FLASH
  initFlash();
//WriteDataToFlash();
//ReadDataFromFlash();
#endif
#ifdef USE_ETHERNET
  initEthernet();
#endif
  tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
  tpFill(0);
}

void loop()
{
  // put your main code here, to run repeatedly:
  byte inChar;
  inChar = Serial.read();
#ifdef USE_ETHERNET
  if (inChar == 'r')
  {
    Serial.println(">>>>>>>>>>>>>READ FTP >>>>>>>>>>>>");
    if (doFTP(fileName))
      Serial.println(F("FTP OK"));
    else
      Serial.println(F("FTP FAIL"));
  }
#endif
  if (inChar == 'f')
  {
    Serial.println(">>>>>>>>>>>>>READ FLS >>>>>>>>>>>>");
    ReadDataFromFlash(24);
  }
#ifdef IMAGE_PROCESS
  if (inChar == 'c')
  {
    Serial.println(">>>>>>>>>>>>>Convert data >>>>>>>>>>>>");
    converTo1BBp();
    ImgLoadStatus = tpLoadBMP((uint8_t *)sourceBuf, 1, 0, 0);
    if (ImgLoadStatus != 0)
    {
      Serial.println("tpLoadBMP error!");
      /*while (1)
      {
      };*/
    }
    else
    {
      Serial.println("tpLoadBMP OK!");
    }
  }

#endif
#ifdef USE_PRINTER
  if (inChar == 'p')
  {
    downloadStatus = true;
  }
  if (downloadStatus)
  {
    //ReadDataFromFlash(24);
  }
  printPicture();
#endif
}
