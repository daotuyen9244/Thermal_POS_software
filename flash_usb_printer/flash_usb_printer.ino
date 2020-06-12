
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
//ReadDataFromFlash(1);
#endif
#ifdef USE_ETHERNET
  initEthernet();
#endif
  //tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
  //tpFill(0);
  getFreeRAM();
}

void loop()
{
  // put your main code here, to run repeatedly:
  byte inChar;
  inChar = Serial.read();

#ifdef USE_ETHERNET
  if (inChar == 'r')
  {
    Serial.println(F(">>>>>>>>>>>>>READ FTP >>>>>>>>>>>>"));
    if (doFTP(fileName))
      Serial.println(F("FTP OK"));
    else
      Serial.println(F("FTP FAIL"));
      getFreeRAM();
  }
#endif
  if (inChar == 'f')
  {
    Serial.println(F(">>>>>>>>>>>>>READ FLS >>>>>>>>>>>>"));
    ReadDataFromFlash(24);
    getFreeRAM();
  }
  if (inChar == 's')
  {
    getFreeRAM();
    WriteDataToFlash();
ReadDataFromFlash(24);
  getFreeRAM();
  }
  if (inChar == 'l')
  {
    ReadDataFromFlash(1);
  }
  if (inChar == 'd')
  {
    ImgLoadStatus = tpLoadBMP((uint8_t *)sourceBuf, 1, 0, 0);
    if (ImgLoadStatus != 0)
    {
      Serial.println(F("tpLoadBMP error!"));
      /*while (1)
      {
      };*/
    }
    else
    {
      Serial.println(F("tpLoadBMP OK!"));
    }
  }
#ifdef IMAGE_PROCESS
  if (inChar == 'c')
  {
    Serial.println(F(">>>>>>>>>>>>>Convert data >>>>>>>>>>>>"));
    getFreeRAM();
    converTo1BBp();
    getFreeRAM();
    tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
    tpFill(0);
    ImgLoadStatus = tpLoadBMP((uint8_t *)sourceBuf, 1, 0, 0);
    if (ImgLoadStatus != 0)
    {
      Serial.println(F("tpLoadBMP error!"));
      /*while (1)
      {
      };*/
    }
    else
    {
      Serial.println(F("tpLoadBMP OK!"));
    }
    getFreeRAM();
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
