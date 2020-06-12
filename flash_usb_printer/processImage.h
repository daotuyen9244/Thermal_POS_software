#define WIDTH 384
#define HEIGHT 40
#define byte_per_page 528
#define CPRINT 10

#include "M0USBPrinter.h"
#include "M0_POS_Printer.h"
#include <SPI.h>
#include <Ethernet.h>
/// USB printer
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
bool downloadStatus = false;
// End printer
//encode bmp
unsigned char dataIn[24];
unsigned char dataReturn = 0;
//unsigned char color_table[8] = {0, 0, 0, 0, 255, 255, 255, 0};
uint8_t sourcePrint[((HEIGHT / 8)) * WIDTH]; // Imge = ucBuf *CPRINT ===> ex: img size 192x128 ==> (print img size 192x1 )* 16
uint8_t sourceBuf[((HEIGHT / 8)) * WIDTH + 62];
///////////////
static int tp_wrap, bb_pitch;
static int bb_width, bb_height; // back buffer width and height in pixels
static uint8_t *pBackBuffer = NULL;
bool ImgLoadStatus = false;

int counterdataProcess = 0;
long counterOfFile = 0;
byte counterForPC = 0;
long counterRef = 0;
long maxLength24bbp = WIDTH * HEIGHT * 3 + 54; // 54 byte header + 3(RGB)*WIDTH*HEIGHT
#define Pin_rst 5
// this must be unique
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67};

// change to your network settings
IPAddress ip(192, 168, 0, 10);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// change to your server
IPAddress server(192, 168, 0, 12);

EthernetClient client;
EthernetClient dclient;

const String ftp_account = "indruino";
const String ftp_passwords = "2019";
String fileName = "";
//Key variable
char outBuf[128];
char outCount;
////////////////
byte doFTP(String fileName);
byte eRcv();
void efail();
void processGetData();
void printPicture()
{
  //Download data from FTP Server Done --> Printer
  if (downloadStatus)
  {
    // Make sure USB printer found and ready
    Serial.println(F("printer begin>>>>>>>>>>>>>>>"));
    if (uprinter.isReady())
    {
      printer.begin();
      Serial.println(F("Init ESC POS printer"));
    }
    printer.reset();
    printer.setDefault();
    printer.printPicture(WIDTH, HEIGHT, sourcePrint, bb_pitch);
    downloadStatus = false;
  }
}
int tpLoadBMP(uint8_t *pBMP, int bInvert, int iXOffset, int iYOffset)
{
  int16_t i16;
  int iOffBits; // offset to bitmap data
  int iPitch;
  int16_t cx, cy, x, y;
  uint8_t *d, *s, pix;
  uint8_t srcmask, dstmask;
  uint8_t bFlipped = false;

  i16 = pBMP[0] | (pBMP[1] << 8);
  if (i16 != 0x4d42) // must start with 'BM'
    return -1;       // not a BMP file
  if (iXOffset < 0 || iYOffset < 0)
    return -1;
  cx = pBMP[18] + (pBMP[19] << 8);
  cy = pBMP[22] + (pBMP[23] << 8);
  if (cy > 0) // BMP is flipped vertically (typical)
    bFlipped = true;
  if (cx + iXOffset > bb_width || cy + iYOffset > bb_height) // too big
    return -1;
  i16 = pBMP[28] + (pBMP[29] << 8);
  if (i16 != 1) // must be 1 bit per pixel
    return -1;
  iOffBits = pBMP[10] + (pBMP[11] << 8);
  iPitch = (cx + 7) >> 3;         // byte width
  iPitch = (iPitch + 3) & 0xfffc; // must be a multiple of DWORDS

  if (bFlipped)
  {
    iOffBits += ((cy - 1) * iPitch); // start from bottom
    iPitch = -iPitch;
  }
  else
  {
    cy = -cy;
  }

  // Send it to the gfx buffer
  for (y = 0; y < cy; y++)
  {
    s = &pBMP[iOffBits + (y * iPitch)]; // source line
    d = &pBackBuffer[((iYOffset + y) * bb_pitch) + iXOffset / 8];
    srcmask = 0x80;
    dstmask = 0x80 >> (iXOffset & 7);
    pix = *s++;
    if (bInvert)
      pix = ~pix;
    for (x = 0; x < cx; x++) // do it a bit at a time
    {
      if (pix & srcmask)
        *d |= dstmask;
      else
        *d &= ~dstmask;
      srcmask >>= 1;
      if (srcmask == 0) // next pixel
      {
        srcmask = 0x80;
        pix = *s++;
        if (bInvert)
          pix = ~pix;
      }
      dstmask >>= 1;
      if (dstmask == 0)
      {
        dstmask = 0x80;
        d++;
      }
    } // for x
  }   // for y
  return 0;
} /* tpLoadBMP() */
void tpSetBackBuffer(uint8_t *pBuffer, int iWidth, int iHeight)
{
  pBackBuffer = pBuffer;
  bb_width = iWidth;
  bb_height = iHeight;
  bb_pitch = (iWidth + 7) >> 3;
  Serial.print(F("bb_pitch: "));
  Serial.println(bb_pitch);
} /* tpSetBackBuffer() */
void tpFill(unsigned char ucData)
{
  if (pBackBuffer != NULL)
    memset(pBackBuffer, ucData, bb_pitch * bb_height);
} /* tpFill() */

unsigned char DataConvert()
{
  byte R, G, B;
  //bit 1
  unsigned char gray = 0;
  unsigned char dt = 0;
  B = dataIn[0];
  G = dataIn[1];
  R = dataIn[2];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7 << 7);
  dt = dt | gray;

  //bit 2
  B = dataIn[3];
  G = dataIn[4];
  R = dataIn[5];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7 << 6);
  dt = dt | gray;

  //bit 3
  B = dataIn[6];
  G = dataIn[7];
  R = dataIn[8];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7 << 5);
  dt = dt | gray;

  //bit 4
  B = dataIn[9];
  G = dataIn[10];
  R = dataIn[11];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7 << 4);
  dt = dt | gray;

  //bit 5
  B = dataIn[12];
  G = dataIn[13];
  R = dataIn[14];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7 << 3);
  dt = dt | gray;

  //bit 6
  B = dataIn[15];
  G = dataIn[16];
  R = dataIn[17];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7 << 2);
  dt = dt | gray;

  //bit 7
  B = dataIn[18];
  G = dataIn[19];
  R = dataIn[20];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7 << 1);
  dt = dt | gray;

  //bit 8
  B = dataIn[21];
  G = dataIn[22];
  R = dataIn[23];
  gray = (unsigned char)(0.11f * B + 0.59 * G + 0.3f * R);
  gray = (unsigned char)(gray >> 7);
  dt = dt | gray;
  return dt;
}
void makingMiniPicture(unsigned char *buff, int iWidth, int iHeight)
{
  //384x40
  byte headerData[62] = {
      0x42, 0x4D, 0xCC, 0xCC, 0x3E, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00,
      0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x28, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
      0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
      0xFF, 0x00};
  for (byte i = 0; i < 62; i++)
  {
    sourceBuf[i] = headerData[i];
  }
}
void initEthernet()
{
  pinMode(Pin_rst, OUTPUT);
  digitalWrite(Pin_rst, LOW);
  delay(1000);
  digitalWrite(Pin_rst, HIGH);
  delay(1000);
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  Serial.println(F("Ethernet connected"));
  Serial.println(F("IP address: "));
  Serial.println(Ethernet.localIP());
  Serial.println(F("Ready. Press r or f ,t"));
}
byte doFTP(String fileName)
{
  if (client.connect(server, 21))
  {
    Serial.println(F("Command connected"));
  }
  else
  {
    Serial.println(F("Command connection failed"));
    return 0;
  }
  if (!eRcv())
    return 0;
  Serial.println("Send USER");
  client.write("USER ");
  client.println(ftp_account);
  if (!eRcv())
    return 0;
  Serial.println("Send PASSWORD");
  client.write("PASS ");
  client.println(ftp_passwords);
  if (!eRcv())
    return 0;
  client.println(F("SYST"));
  if (!eRcv())
    return 0;
  client.println(F("Type I"));
  if (!eRcv())
    return 0;
  client.println(F("PASV"));
  if (!eRcv())
    return 0;
  char *tStr = strtok(outBuf, "(,");
  int array_pasv[6];
  for (int i = 0; i < 6; i++)
  {
    tStr = strtok(NULL, "(,");
    array_pasv[i] = atoi(tStr);
    if (tStr == NULL)
    {
      Serial.println(F("Bad PASV Answer"));
    }
  }
  unsigned int hiPort, loPort;
  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;
  Serial.print(F("Data port: "));
  hiPort = hiPort | loPort;
  Serial.println(hiPort);
  if (dclient.connect(server, hiPort))
  {
    Serial.println(F("Data connected"));
  }
  else
  {
    Serial.println(F("Data connection failed"));
    client.stop();
    return 0;
  }
  client.print(F("RETR "));
  client.println(fileName);
  if (!eRcv())
  {
    dclient.stop();
    return 0;
  }
  processGetData();
  dclient.println();
  dclient.stop();
  makingMiniPicture(sourceBuf, WIDTH, HEIGHT); // making header file for 1bbp bitmap
  tpSetBackBuffer(sourcePrint, WIDTH, HEIGHT);
  tpFill(0);
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
  }
  Serial.println(F("Data disconnected"));
  if (!eRcv())
    return 0;
  client.println(F("QUIT"));
  if (!eRcv())
    return 0;
  client.stop();
  Serial.println(F("Command disconnected"));
  return 1;
}

byte eRcv()
{
  byte respCode;
  byte thisByte;
  byte xfer;
  while (!client.available())
    delay(1);
  respCode = client.peek();
  outCount = 0;
  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
    if (outCount < 127)
    {
      outBuf[outCount] = thisByte;
      outCount++;
      outBuf[outCount] = 0;
    }
  }
  if (respCode >= '4')
  {
    efail();
    return 0;
  }
  return 1;
}

void efail()
{
  byte thisByte = 0;
  client.println(F("QUIT"));
  while (!client.available())
    delay(1);
  while (client.available())
  {
    thisByte = client.read();
    Serial.write(thisByte);
  }
  client.stop();
  Serial.println(F("Command disconnected"));
}
void processGetData()
{
  downloadStatus = false;
  Serial.println(F("Reading"));
  counterdataProcess = 62;
  counterOfFile = 0;
  counterRef = 0;
  while (dclient.connected())
  {
    while (dclient.available())
    {
      char c = dclient.read();
      counterOfFile++;
      if (counterOfFile <= maxLength24bbp)
      {
        if (counterOfFile > 54)
        //if (counterOfFile > 62) // 54 byte header + 8 byte color
        {
          dataIn[counterForPC++] = c;
          if (counterForPC > 23) // counter 8 byte
          {
            if (counterRef < (WIDTH * (HEIGHT / 8)))
            {
              dataReturn = DataConvert();
              sourceBuf[counterdataProcess] = dataReturn;
              counterdataProcess++;
            }
            else
            {
              counterOfFile = 0;
              counterRef = 0;
              dclient.stop();
            }
            counterRef++;
            counterForPC = 0;
          }
          if (counterOfFile % 2000 == 0)
          {
            Serial.print(".");
          }
        }
      }
      else
      {
        dclient.stop();
      }
    }
    downloadStatus = true;
  }
  Serial.println("done.");
  Serial.print("counterOfFile: ");
  Serial.println(counterOfFile);
  Serial.print("counterRef: ");
  Serial.println(counterRef);
  counterOfFile = 0;
  counterRef = 0;
}