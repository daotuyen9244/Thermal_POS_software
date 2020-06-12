
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
  myusb.Task();
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
#ifdef USE_DEMO
  if (inChar == 'f')
  {
    Serial.println(F(">>>>>>>>>>>>>READ FLS >>>>>>>>>>>>"));
    ReadDataFromFlash(1);
    getFreeRAM();
  }

  if (inChar == 's')
  {
    WriteDataToFlash();
  }
#endif
  if (inChar == 'q') /// clear data into flash
  {
    ClearDataFlash();
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
    ImgLoadStatus = tpLoadBMP(sourceBuf, 1, 0, 0);
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

#endif
  if ((inChar >= '0') && (inChar <= '9'))
  {
    /*
  //for(byte i=0;i<9;i++)
  //{

 counterForPage = 0;   // count: 0 ->calculatePage
 counterForByte = 0;   // counter all data save to flash
 counterFor1bffer = 0;
  Serial.print(" moi vua nhap vao: ");
  Serial.println(inChar);
  fileName= String(inChar-48) +".bmp";
  Serial.print("fileName: ");
  Serial.println(fileName);

  Serial.println(F(">>>>>>>>>>>>>READ FTP >>>>>>>>>>>>"));
    if (doFTP(fileName))     // step 1
    {
      Serial.println(F("FTP OK"));

      ReadDataFromFlash(1); // step 2
      
      Serial.println(F(">>>>>>>>>>>>>Convert data >>>>>>>>>>>>"));// step 3
    converTo1BBp();
    tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
    tpFill(0);
    ImgLoadStatus = tpLoadBMP(0, 1, 0, 0);
    ImgLoadStatus = tpLoadBMP(sourceBuf, 1, 0, 0);
    if (ImgLoadStatus != 0)
    {
      Serial.println(F("tpLoadBMP error!"));
    }
    else
    {
      Serial.println(F("tpLoadBMP OK!"));
      downloadStatus = true;// step 4
      printPicture();
      tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
      tpFill(0);
    }
    
    

    }
    else
    {
      Serial.println(F("FTP FAIL"));
    }
    }*/

    //}
    for (byte i = 0; i < 9; i++)
    {

      counterForPage = 0; // count: 0 ->calculatePage
      counterForByte = 0; // counter all data save to flash
      counterFor1bffer = 0;
      Serial.print(" moi vua nhap vao: ");
      Serial.println(inChar);
      fileName = String(i) + ".bmp";
      Serial.print("fileName: ");
      Serial.println(fileName);

      Serial.println(F(">>>>>>>>>>>>>READ FTP >>>>>>>>>>>>"));
      if (doFTP(fileName)) // step 1
      {
        Serial.println(F("FTP OK"));
        //ReadDataFromFlash(1); // step 2

        //Serial.println(F(">>>>>>>>>>>>>Convert data >>>>>>>>>>>>"));// step 3
        //converTo1BBp();
        makingMiniPicture(sourceBuf, WIDTH, HEIGHT); // making header file for 1bbp bitmap
        tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
        tpFill(0);
        //ImgLoadStatus = tpLoadBMP(0, 1, 0, 0);
        ImgLoadStatus = tpLoadBMP(sourceBuf, 1, 0, 0);
        if (ImgLoadStatus != 0)
        {
          Serial.println(F("tpLoadBMP error!"));
        }
        else
        {
          Serial.println(F("tpLoadBMP OK!"));
          downloadStatus = true; // step 4
          printPicture();
          tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
          tpFill(0);
        }
      }
      else
      {
        Serial.println(F("FTP FAIL"));
      }
    }
  }
}
