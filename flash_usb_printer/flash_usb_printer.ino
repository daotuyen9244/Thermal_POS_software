#include <SPI.h>
#include <SoftSPIB.h>
#include "DataFlash.h"
#include "processImage.h"
#ifdef USE_PRINTER
#include "M0USBPrinter.h"
#include "M0_POS_Printer.h"

class PrinterOper : public USBPrinterAsyncOper
{
public:
  uint8_t OnInit(USBPrinter *pPrinter);
};

uint8_t PrinterOper::OnInit(USBPrinter *pPrinter)
{
  Serial.println(F("USB Printer OnInit"));
  Serial.print(F("Bidirectional="));
  Serial.println(pPrinter->isBidirectional());
  return 0;
}

USBHost myusb;
PrinterOper AsyncOper;
USBPrinter uprinter(&myusb, &AsyncOper);
ESC_POS_Printer printer(&uprinter);
#endif
DataFlash dataflash;
// 192x128

long calculatePage24bbp = 0;
long calculatePage1bbp = 0;
long calculateSize24bbp = 0;
long calculateSize1bbp = 0;
long counterForPage = 0; // count: 0 ->calculatePage
long counterForByte = 0; // count: 0 ->528
static byte _buffer = 0;
void setup()
{
  /* Initialize SPI */
  //SPI.begin();

  /* Let's wait 1 second, allowing use to press the serial monitor button :p */
  delay(1000);

  /* Initialize dataflash */
  dataflash.setup(2, 255, 255);

  delay(10);

  dataflash.begin();

  /* Set baud rate for serial communication */
  Serial.begin(115200);
  /// storgare data to flash
  calculateSize24bbp = (WIDTH * HEIGHT * 3 + 54);
  calculateSize1bbp = (WIDTH * HEIGHT / 8 + 62);
  calculatePage24bbp = calculateSize24bbp / byte_per_page + 1;
  calculatePage1bbp = calculateSize1bbp / byte_per_page + 1;

  Serial.println("Information:");
  Serial.print("bitmap 24bbp: ");
  Serial.print(WIDTH);
  Serial.print("x");
  Serial.print(HEIGHT);
  Serial.print(",");
  Serial.print("Size:");
  Serial.print(WIDTH * HEIGHT * 3 + 54);
  Serial.print(" ,calculatePage24bbp:");
  Serial.println(calculatePage24bbp);
  Serial.print("bitmap 1bbp: ");
  Serial.print(WIDTH);
  Serial.print("x");
  Serial.print(HEIGHT);
  Serial.print(",");
  Serial.print("Size:");
  Serial.print(WIDTH * HEIGHT / 8 + 62);
  Serial.print(" ,calculatePage1bbp:");
  Serial.println(calculatePage1bbp);
  func1();
  func2();
}
void func1() // fill and read 1 byte
{
  byte _data = 0;
  int _counter = 0;
  counterForByte = 0;
  counterForPage = 0;
  for (byte i = 0; i < calculatePage1bbp; i++)
  {
    if (counterForByte < calculateSize1bbp)
    {
      dataflash.bufferWrite(0, 0); // buferr 0 --> offset 0
      for (int k = 0; k < byte_per_page; k++)
      {

        _data = logo_bmp[counterForByte];
        dataflash.Stransfer(_data);
        counterForByte++;
      }

      dataflash.bufferToPage(0, i); //buffer --> page
      delay(100);
    }
  }
  Serial.print("counterForByte: ");
  Serial.print(counterForByte);
  Serial.print("\t");
  Serial.print("counterForPage: ");
  Serial.println(counterForPage);
}
void func2()
{
  byte _data = 0;
  int _counter = 0;
  counterForByte = 0;
  counterForPage = 0;
  byte comp = 0;
  for (byte i = 0; i < calculatePage1bbp; i++)
  {
    //dataflash.pageRead(i, 0);
    dataflash.pageToBuffer(i, 1);
    dataflash.bufferRead(1, 0);
    for (int k = 0; k < byte_per_page; k++)
    {
      if (counterForByte < calculateSize1bbp)
      {
        _data = dataflash.Stransfer(0xff);

        if (_data != logo_bmp[byte_per_page * i + k])
        {
          comp = 1;
        }
        Serial.print("counterForByte: ");
        Serial.print(counterForByte);
        Serial.print("\t");
        Serial.print(_data);
        Serial.print("\t");
        Serial.println(logo_bmp[counterForByte]);
        counterForByte++;
      }
    }
  }

  Serial.print("comp: ");
  Serial.println(comp);
}
void loop()
{
  // put your main code here, to run repeatedly:
  byte inChar;
  inChar = Serial.read();
#ifdef USE_FTP
  //if (inChar == 'f')
  {
    if (ReadFromFTP())
      Serial.println(F("FTP OK"));
    else
      Serial.println(F("FTP FAIL"));
  }
#endif
  if (inChar == 'p')
  {
    inChar = 0;
    downloadStatus = true;
  }

#ifdef USE_PRINTER
  myusb.Task();
  //Download data from FTP Server Done --> Printer
  if (downloadStatus)
  {
    // Make sure USB printer found and ready
    Serial.println(F("printer begin>>>>>>>>>>>>>>>"));
#ifdef USE_PRINTER
    if (uprinter.isReady())
    {
      printer.begin();
#ifdef USE_DEBUG
      Serial.println(F("Init ESC POS printer"));
    }
#endif
    printer.reset();
    printer.setDefault();
    //#ifdef USE_DECODE
    //printer.printPicture(WIDTH, HEIGHT, sourcePrint, bb_pitch);
    //#endif
#endif
    downloadStatus = false;
  }
#endif
}
//void fillData(unsigned char buffer[], int bufferSize, int page)
void fillData(unsigned char *buffer, int bufferSize, int page)
{
  dataflash.bufferWrite(_buffer, 0);
  for (uint16_t j = 0; j < bufferSize; j++)
  {
    dataflash.Stransfer(buffer[j]);
  }
  dataflash.bufferToPage(_buffer, page);
  _buffer ^= 1;
}
void readBufferData(unsigned char *buffer, int bufferSize, int page)
{

  dataflash.pageRead(page, 0);      // page --> offset
  dataflash.bufferRead(_buffer, 0); // buffer --> offset
  for (int i = 0; i < bufferSize; i++)
  {
    byte data = 0;
    data = dataflash.Stransfer(0xff);
    buffer[i] = data;
    //Serial.print(data, HEX);
  }
  _buffer ^= 1;
}
