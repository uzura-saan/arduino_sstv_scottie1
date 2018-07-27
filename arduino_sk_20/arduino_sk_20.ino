/********************************************************
  SSTV WIRELESS CAMERA @uzura-saan
  Scottie1 Format (320x256, COLOR)
  2018.07.27 v2.3
  ARDUINO "DUE" + ETHERNET SHIELD R3 + AD9850 DDS
  ETHERNET SHIELD ACTS JUST AS A "SD CARD SHIELD"
  --- THIS CODE IS IN THE PUBLIC DOMAIN --- 
  
  JPEG DECODE SECTION IS BASED ON MAKOTO KURAUCHI'S WORK
  https://github.com/MakotoKurauchi/JPEGDecoder
/********************************************************/

#include <arduino.h>
#include <SPI.h>
#include <SD.h>
#include <DueTimer.h>
#include "JPEGDecoder.h"

File myFile1;
#define chipSelect 4

#define  CLK   6
#define  FQ    7
#define  DATA  8
#define  RST   9

String string1 = String("S");
String string2 = String("");
String string3 = String(".JPG");
String string4 = String("");
char fileName[8];
byte fileNumber;

volatile byte sTimes;
volatile byte sSq;
volatile byte gbrCnt;
volatile int  line;
volatile int  ti;
volatile long syncTime;
volatile byte lineBufR[320];
volatile byte lineBufG[320];
volatile byte lineBufB[320];
volatile byte lineBufE[320];

const boolean vox = 1; //VOX TONE ENABLE

char charId[13] = "JI3BNB-SSTV-"; // ***** INFORMATION HEADER: MAX 12 CAHARCTERS *****

//FONTS
const uint8_t fonts[43][11] = {
        {0x00, 0x18, 0x24, 0x62, 0x62, 0x62, 0x7E, 0x62, 0x62, 0x62, 0x00}, //00: A
        {0x00, 0x7C, 0x32, 0x32, 0x32, 0x3C, 0x32, 0x32, 0x32, 0x7C, 0x00}, //01: B
        {0x00, 0x3C, 0x62, 0x62, 0x60, 0x60, 0x60, 0x62, 0x62, 0x3C, 0x00}, //02: C
        {0x00, 0x7C, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x7C, 0x00}, //03: D
        {0x00, 0x7E, 0x60, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x7E, 0x00}, //04: E
        {0x00, 0x7E, 0x60, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x00}, //05: F
        {0x00, 0x3C, 0x62, 0x62, 0x60, 0x60, 0x66, 0x62, 0x62, 0x3C, 0x00}, //06: G
        {0x00, 0x62, 0x62, 0x62, 0x62, 0x7E, 0x62, 0x62, 0x62, 0x62, 0x00}, //07: H
        {0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, //08: I
        {0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x4C, 0x4C, 0x4C, 0x38, 0x00}, //09: J
        {0x00, 0x62, 0x64, 0x68, 0x70, 0x68, 0x64, 0x62, 0x62, 0x62, 0x00}, //10: K
        {0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, //11: L
        {0x00, 0x42, 0x62, 0x76, 0x6A, 0x62, 0x62, 0x62, 0x62, 0x62, 0x00}, //12: M
        {0x00, 0x42, 0x62, 0x72, 0x6A, 0x66, 0x62, 0x62, 0x62, 0x62, 0x00}, //13: N
        {0x00, 0x3C, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x3C, 0x00}, //14: O
        {0x00, 0x7C, 0x62, 0x62, 0x62, 0x7C, 0x60, 0x60, 0x60, 0x60, 0x00}, //15: P
        {0x00, 0x3C, 0x62, 0x62, 0x62, 0x62, 0x62, 0x6A, 0x6A, 0x3C, 0x08}, //16: Q
        {0x00, 0x7C, 0x62, 0x62, 0x62, 0x7C, 0x68, 0x64, 0x62, 0x62, 0x00}, //17: R
        {0x00, 0x3C, 0x62, 0x60, 0x60, 0x3C, 0x06, 0x06, 0x46, 0x3C, 0x00}, //18: S
        {0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, //19: T
        {0x00, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x3C, 0x00}, //20: U
        {0x00, 0x62, 0x62, 0x62, 0x62, 0x62, 0x62, 0x22, 0x14, 0x08, 0x00}, //21: V
        {0x00, 0x62, 0x62, 0x62, 0x62, 0x62, 0x6A, 0x76, 0x62, 0x42, 0x00}, //22: W
        {0x00, 0x42, 0x62, 0x74, 0x38, 0x1C, 0x2E, 0x46, 0x42, 0x42, 0x00}, //23: X
        {0x00, 0x42, 0x62, 0x74, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, //24: Y
        {0x00, 0x7E, 0x06, 0x0E, 0x0C, 0x18, 0x30, 0x70, 0x60, 0x7E, 0x00}, //25: Z
        {0x00, 0x3C, 0x62, 0x62, 0x66, 0x6A, 0x72, 0x62, 0x62, 0x3C, 0x00}, //26: 0
        {0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, //27: 1
        {0x00, 0x3C, 0x46, 0x06, 0x06, 0x1C, 0x20, 0x60, 0x60, 0x7E, 0x00}, //28: 2
        {0x00, 0x3C, 0x46, 0x06, 0x06, 0x1C, 0x06, 0x06, 0x46, 0x3C, 0x00}, //29: 3
        {0x00, 0x0C, 0x1C, 0x2C, 0x4C, 0x4C, 0x7E, 0x0C, 0x0C, 0x0C, 0x00}, //30: 4
        {0x00, 0x7E, 0x60, 0x60, 0x60, 0x7C, 0x06, 0x06, 0x46, 0x3C, 0x00}, //31: 5
        {0x00, 0x3C, 0x62, 0x60, 0x60, 0x7C, 0x62, 0x62, 0x62, 0x3C, 0x00}, //32: 6
        {0x00, 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00}, //33: 7
        {0x00, 0x3C, 0x62, 0x62, 0x62, 0x3C, 0x62, 0x62, 0x62, 0x3C, 0x00}, //34: 8
        {0x00, 0x3C, 0x46, 0x46, 0x46, 0x3E, 0x06, 0x06, 0x46, 0x3C, 0x00}, //35: 9
        {0x00, 0x00, 0x02, 0x06, 0x0E, 0x1C, 0x38, 0x70, 0x60, 0x40, 0x00}, //36: /
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00}, //37: -
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x00}, //38: .
        {0x00, 0x3C, 0x46, 0x06, 0x06, 0x0C, 0x10, 0x00, 0x30, 0x30, 0x00}, //39: ?
        {0x00, 0x18, 0x18, 0x18, 0x18, 0x10, 0x10, 0x00, 0x18, 0x18, 0x00}, //40: !
        {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00}, //41: :
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  //42: space
};

void shortPulse (char PIN)
{
        digitalWrite(PIN, 1);
        digitalWrite(PIN, 0);
}

void setFreq(double freq)
{
        //--calculate
        int32_t d_Phase = freq * pow(2, 32) / 125000000;
        //--send first 32bit
        for (int i=0; i<32; i++, d_Phase>>=1)
        {
                if(d_Phase & 1 == 1)
                {
                        digitalWrite(DATA, HIGH); //--data
                }
                else
                {
                        digitalWrite(DATA, LOW); //--data
                }
                shortPulse(CLK);
        }
        //--send rest 8bit
        digitalWrite(DATA, LOW); //--data
        for (int i=0; i<8; i++)
        {
                shortPulse(CLK);
        }
        //--finish
        shortPulse(FQ);
}

void setup()
{
        char ch;
        
        Serial.begin(9600);
        Serial1.begin(38400); //Camera Serial
        delay(300);
        while(Serial.available())
        {
                ch = Serial.read(); //flush
        }
        while(Serial1.available())
        {
                ch = Serial1.read(); //flush
        }
        Serial.println("SERIAL READY");
        
        pinMode(10, OUTPUT);
        pinMode(31, OUTPUT); //ERROR LAMP
        pinMode(33, OUTPUT); //SHOT LAMP
        digitalWrite(31, LOW);
        digitalWrite(33, LOW);
        
        if (!SD.begin(chipSelect))
        {
                Serial.println("ERROR: INSERT SD CARD AND RESTART");
                digitalWrite(31, HIGH);
                return ;
        }
        Serial.println("SD CARD INITIALIZED");
        Serial.println();
        
        pinMode(47, INPUT);
        pinMode(49, INPUT);
        digitalWrite(47, 1); //INTERNAL PULL UP
        digitalWrite(49, 1); //INTERNAL PULL UP
        
        pinMode(CLK, OUTPUT);
        pinMode(FQ, OUTPUT);
        pinMode(DATA, OUTPUT);
        pinMode(RST, OUTPUT);
        //--dds reset
        shortPulse(RST);
        shortPulse(CLK);
        //--change mode
        shortPulse(FQ);
        
        //--cam reset
        Serial1.print((char)0x56);
        Serial1.print((char)0x00);
        Serial1.print((char)0x26);
        Serial1.print((char)0x00);
        delay(2500);
        
        while(!Serial1.available()){ }
        while(Serial1.available())
        {
                Serial.print(Serial1.read(), HEX);
                Serial.print(" ");
        }
        Serial.println();
        
        //--cam setSize
        Serial1.print((char)0x56);
        Serial1.print((char)0x00);
        Serial1.print((char)0x54);
        Serial1.print((char)0x01);
        Serial1.print((char)0x11); //320*240:
        delay(100);
        
        while(!Serial1.available()){ }
        while(Serial1.available())
        {
                Serial.print(Serial1.read(), HEX);
                Serial.print(" ");
        }
        Serial.println();
        
        //--cam compRatio
        Serial1.print((char)0x56);
        Serial1.print((char)0x00);
        Serial1.print((char)0x31);
        Serial1.print((char)0x05);
        Serial1.print((char)0x01);
        Serial1.print((char)0x01);
        Serial1.print((char)0x12);
        Serial1.print((char)0x04);
        Serial1.print((char)0x36); //SMALLER VALUE: LESS COMPRESSED (DEFAULT: 0x36)
        delay(100);
        
        while(!Serial1.available()){ }
        while(Serial1.available())
        {
                Serial.print(Serial1.read(), HEX);
                Serial.print(" ");
        }
        Serial.println();
        
        //--cam motionDetectEnable
        Serial1.print((char)0x56);
        Serial1.print((char)0x00);
        Serial1.print((char)0x42);
        Serial1.print((char)0x03);
        Serial1.print((char)0x00);
        Serial1.print((char)0x01);
        Serial1.print((char)0x01);
        delay(100);
        
        while(!Serial1.available()){ }
        while(Serial1.available())
        {
                Serial.print(Serial1.read(), HEX);
                Serial.print(" ");
        }
        Serial.println();
        
        Serial.println("JPEG CAMERA INITIALIZED");
        Serial.println();
        
        Timer1.attachInterrupt(timer1_interrupt).start(430); // ***** 432(uS/px) +/- SLANT ADJUST *****
        delay(100);
}

void timer1_interrupt(void)
{
        if(sSq == 3)
        {
                if(ti < 320)
                {
                             if(gbrCnt == 0){setFreq(1500 + 3.13 * lineBufG[ti]);}
                        else if(gbrCnt == 1){setFreq(1500 + 3.13 * lineBufB[ti]);}
                        else if(gbrCnt == 2){setFreq(1500 + 3.13 * lineBufE[ti]);}
                }
                else if(ti == 320)
                {
                             if(gbrCnt == 0){setFreq(1500);}
                        else if(gbrCnt == 1){setFreq(1200);}
                        else if(gbrCnt == 2){setFreq(1500);}
                        syncTime = micros();
                        sSq = 4;
                }
                ti++;
        }
}

void jpegTake(void)
{
        byte start_addr_m;
        byte start_addr_l;
        int  byteCount;
        byte incomingByte;
        byte last2bytes[2];
        boolean jpegEnd;
        boolean err;
        
        Serial.println("NEW FILE");
        
        //--cam takePhoto
        digitalWrite(33, HIGH);
        Serial1.print((char)0x56);
        Serial1.print((char)0x00);
        Serial1.print((char)0x36);
        Serial1.print((char)0x01);
        Serial1.print((char)0x00);
        delay(100);
        
        while(!Serial1.available()){ }
        digitalWrite(33, LOW);
        while(Serial1.available())
        {
                Serial.print(Serial1.read(), HEX);
                Serial.print(" ");
        }
        Serial.println();
        
        //--cam readSize
        Serial1.print((char)0x56);
        Serial1.print((char)0x00);
        Serial1.print((char)0x34);
        Serial1.print((char)0x01);
        Serial1.print((char)0x00);
        delay(100);
        
        while(!Serial1.available()){ }
        while(Serial1.available())
        {
                Serial.print(Serial1.read(), HEX);
                Serial.print(" ");
        }
        Serial.println();
        
        //--cam readContent
        start_addr_m = 0x00;
        start_addr_l = 0x00;
        jpegEnd = false;
        
        //--jpeg fileSave
        string2 = String(fileNumber);
        string4 = String(string1 + string2 + string3);
        string4.toCharArray(fileName, 8);
        
        err = false;
        myFile1 = SD.open(fileName, FILE_WRITE | O_TRUNC);
        if(myFile1)
        {
        
                while(!jpegEnd)
                {
                        /*
                        Serial.print("start_addr: ");
                        Serial.print(start_addr_m, HEX);
                        Serial.print(" ");
                        Serial.print(start_addr_l, HEX);
                        Serial.println();
                        delay(10);
                        */
                        Serial1.print((char)0x56);
                        Serial1.print((char)0x00);
                        Serial1.print((char)0x32);
                        Serial1.print((char)0x0C);
                        Serial1.print((char)0x00);
                        Serial1.print((char)0x0A);
                        Serial1.print((char)0x00);
                        Serial1.print((char)0x00);
                        
                        Serial1.print((char)start_addr_m);  //MM
                        Serial1.print((char)start_addr_l);  //MM
                        
                        Serial1.print((char)0x00);
                        Serial1.print((char)0x00);
                        Serial1.print((char)0x00);  //KK
                        Serial1.print((char)0x20);  //KK (DEC:32)
                        Serial1.print((char)0x00);  //XX
                        Serial1.print((char)0x0A);  //XX
                        
                        while(!Serial1.available()){ }
                        delay(22); //10ms at least
                        
                        byteCount = 0;
                        while(Serial1.available())
                        {
                                incomingByte = Serial1.read();
                                /*
                                Serial.print(incomingByte, HEX);
                                Serial.print(" ");
                                */
                                if(byteCount >=5 && byteCount <=36)
                                {
                                        myFile1.write(incomingByte);
                                        
                                        last2bytes[1] = incomingByte;
                                        if(last2bytes[0] == 0xFF && last2bytes[1] == 0xD9)
                                        {
                                                /*
                                                Serial.println("CAUGHT FFD9!");
                                                */
                                                jpegEnd = true;
                                        }
                                        last2bytes[0] = last2bytes[1];
                                }
                                byteCount++;
                        }
                        /*
                        Serial.println();
                        */
                        if(start_addr_l == 0xE0)
                        {
                                start_addr_l = 0x00;
                                start_addr_m += 0x01;
                        }
                        else
                        {
                                start_addr_l += 0x20; //DEC:32
                        }
                }
        }
        else
        {
                Serial.println("CAN'T OPEN FILE");
                err = true;
        }
        myFile1.close();
        
        if(err == true)
        {
                digitalWrite(31, HIGH);
                while(1);
        }
        else
        {
                Serial.println(fileName);
                Serial.println("JPEG FILE SAVED");
                Serial.println();
        }
        
        //--cam stop
        Serial1.print((char)0x56);
        Serial1.print((char)0x00);
        Serial1.print((char)0x36);
        Serial1.print((char)0x01);
        Serial1.print((char)0x03);
        delay(100);
        
        while(!Serial1.available()){ }
        while(Serial1.available())
        {
                Serial.print(Serial1.read(), HEX);
                Serial.print(" ");
        }
        Serial.println();
}

void jpegDecode(void)
{
        char str[100];
        //char filename[] = "SSTV.JPG";
        uint8 *pImg;
        int x,y,bx,by;
        
        boolean err;
        byte sortBuf[15360]; //320(px)*16(lines)*3(bytes)
        int i,j,k;
        int pxSkip;
        
        // Decoding start
        JpegDec.decode(fileName,0);
        
        // Image Information
        Serial.print("Width     :");
        Serial.println(JpegDec.width);
        Serial.print("Height    :");
        Serial.println(JpegDec.height);
        Serial.print("Components:");
        Serial.println(JpegDec.comps);
        Serial.print("MCU / row :");
        Serial.println(JpegDec.MCUSPerRow);
        Serial.print("MCU / col :");
        Serial.println(JpegDec.MCUSPerCol);
        Serial.print("Scan type :");
        Serial.println(JpegDec.scanType);
        Serial.print("MCU width :");
        Serial.println(JpegDec.MCUWidth);
        Serial.print("MCU height:");
        Serial.println(JpegDec.MCUHeight);
        Serial.println("");
        
        // Serial output
        sprintf(str,"#SIZE,%d,%d",JpegDec.width,JpegDec.height);
        Serial.println(str);
        
        // Image size error
        if(JpegDec.width != 320 || JpegDec.height != 240)
        {
                digitalWrite(31, HIGH);
                Serial.println("ERROR: PICTURE SIZE SHOULD BE 320*240") ;
                while(1);
        }
        
        // Add header - Decode jpeg
        err = false;
        myFile1 = SD.open("RGB.DAT", FILE_WRITE | O_TRUNC);
        if(myFile1)
        {
                // Add header 16(lines)
                for(i = 0; i < 15360; i++)
                {
                        sortBuf[i] = 0xFF;
                }
                for(i = 0; i < 12; i++)
                {
                        byte fontNumber;
                        char ch;
                        ch = charId[i];
                        
                        for(y = 0; y < 11; y++)
                        {
                                for(x = 0; x < 8; x++)
                                {
                                        pxSkip = 16 + (320 * (y + 3)) + (3 * 8 * i) + (3 * x); //Width: x3
                                        
                                        uint8_t mask;
                                        mask = pow(2, 7 - x);
                                        
                                        if(ch >= 65 && ch <= 90) //A to Z
                                        {
                                                fontNumber = ch - 65;
                                        }
                                        else if(ch >= 48 && ch <= 57) //0 to 9
                                        {
                                                fontNumber = ch - 22;
                                        }
                                        else if(ch == '/'){fontNumber = 36;}
                                        else if(ch == '-'){fontNumber = 37;}
                                        else if(ch == '.'){fontNumber = 38;}
                                        else if(ch == '?'){fontNumber = 39;}
                                        else if(ch == '!'){fontNumber = 40;}
                                        else if(ch == ':'){fontNumber = 41;}
                                        else if(ch == ' '){fontNumber = 42;}
                                        else              {fontNumber = 42;}
                                        
                                        if((fonts[fontNumber][y] & mask) != 0)
                                        {
                                                for(j = 0; j < 9; j++)
                                                {
                                                        sortBuf[(3 * pxSkip) + j] = 0x00;
                                                }
                                        }
                                }
                        }
                }
                for(i = 0; i < 15360; i++)
                {
                        myFile1.write(sortBuf[i]);
                }
                
                // Decode jpeg
                i = 0;
                j = 0;
                while(JpegDec.read())
                {
                        pImg = JpegDec.pImage ;
                        
                        for(by=0; by < JpegDec.MCUHeight; by++)
                        {
                                for(bx=0; bx < JpegDec.MCUWidth; bx++)
                                {
                                        x = JpegDec.MCUx * JpegDec.MCUWidth + bx;
                                        y = JpegDec.MCUy * JpegDec.MCUHeight + by;
                                        
                                        if(x < JpegDec.width && y < JpegDec.height)
                                        {
                                                if(JpegDec.comps == 1) // Grayscale
                                                {
                                                        //sprintf(str,"#RGB,%d,%d,%u", x, y, pImg[0]);
                                                        //Serial.println(str);
                                                }
                                                else // RGB
                                                {
                                                        //sprintf(str,"#RGB,%d,%d,%u,%u,%u", x, y, pImg[0], pImg[1], pImg[2]);
                                                        //Serial.println(str);
                                                        
                                                        pxSkip = ((y - (16 * j)) * 320) + x;
                                                        sortBuf[(3 * pxSkip) + 0] = pImg[0];
                                                        sortBuf[(3 * pxSkip) + 1] = pImg[1];
                                                        sortBuf[(3 * pxSkip) + 2] = pImg[2];
                                                        
                                                        i++;
                                                        if(i == 5120) //320(px)x16(lines)
                                                        {
                                                                for(k = 0; k < 15360; k++)
                                                                {
                                                                        myFile1.write(sortBuf[k]);
                                                                }
                                                                i = 0;
                                                                j++; //15(sections)
                                                        }
                                                }
                                        }
                                        pImg += JpegDec.comps ;
                                }
                        }
                }
                //for(;;);
                Serial.println("JPEG FILE DECODED");
                Serial.println();
        }
        else
        {
                Serial.println("CAN'T OPEN FILE");
                err = true;
        }
        myFile1.close();
        
        if(err == true)
        {
                digitalWrite(31, HIGH);
                while(1);
        }
        else
        {
                Serial.println("DAT FILE SAVED");
                Serial.println();
        }
}

void loop()
{
        boolean err;
        boolean head;
        
        int x,y,i,j;
        
        if(sSq == 0)
        {
                byte incomingByte;
                
                setFreq(2); //Mute
                
                while(sSq == 0)
                {
                        if(digitalRead(47) == 0) //TX enabled
                        {
                                if(digitalRead(49) == 0) //Holded
                                {
                                        sTimes = 0;
                                        sSq = 1;
                                }
                                else if(sTimes > 0) //Send 2 times
                                {
                                        sSq = 1;
                                }
                                else
                                {
                                        delay(500);
                                        
                                        //Motion detection
                                        Serial1.print((char)0x56);
                                        Serial1.print((char)0x00);
                                        Serial1.print((char)0x37);
                                        Serial1.print((char)0x01);
                                        Serial1.print((char)0x01); //Start
                                        delay(100);
                                        
                                        while(!Serial1.available()){ }
                                        while(Serial1.available())
                                        {
                                                Serial.print(Serial1.read(), HEX);
                                                Serial.print(" ");
                                        }
                                        Serial.println();
                                        
                                        Serial.println("MOTION DETECTION STARTED");
                                        Serial.println();
                                        
                                        while(sSq == 0)
                                        { 
                                                while(Serial1.available())
                                                {
                                                        incomingByte = Serial1.read();
                                                        if(incomingByte == 0x39)
                                                        {
                                                                Serial.println(incomingByte, HEX);
                                                                Serial.println("MOTION DETECTED!");
                                                                Serial.println();
                                                                sTimes = 2;
                                                                sSq = 1;
                                                        }
                                                }
                                                if(analogRead(3) > 850) //External trigger
                                                {
                                                        Serial.println("A3 TRIGGERED!");
                                                        Serial.println();
                                                        sTimes = 2;
                                                        sSq = 1;
                                                }
                                                
                                                delay(500);
                                                
                                                if(digitalRead(49) == 0) //Holded
                                                {
                                                        sTimes = 0;
                                                        sSq = 1;
                                                }
                                                else if(digitalRead(47) == 1) //TX disabled
                                                {
                                                        sTimes = 0;
                                                        sSq = 0;
                                                        break;
                                                }
                                        }
                                        
                                        //Motion detection
                                        Serial1.print((char)0x56);
                                        Serial1.print((char)0x00);
                                        Serial1.print((char)0x37);
                                        Serial1.print((char)0x01);
                                        Serial1.print((char)0x00); //Stop
                                        delay(100);
                                        
                                        while(!Serial1.available()){ }
                                        while(Serial1.available())
                                        {
                                                Serial.print(Serial1.read(), HEX);
                                                Serial.print(" ");
                                        }
                                        Serial.println();
                                        
                                        Serial.println("MOTION DETECTION STOPPED");
                                        Serial.println();
                                }
                        }
                        else //Disabled
                        {
                                sTimes = 0;
                                delay(5);
                        }
                }
        }
        if(sSq >= 1)
        {
                jpegTake();
                jpegDecode();
                fileNumber++;
                if(fileNumber == 100)
                {
                        fileNumber = 0;
                }
                
                err = false;
                myFile1 = SD.open("RGB.DAT");
                if(myFile1)
                {
                        head = true;
                        while(myFile1.available() || line == 255)
                        {
                                //Header
                                if(head == true)
                                {
                                        //-- VOX TONE --
                                        if(vox == 1)
                                        {
                                                setFreq(1900);
                                                delay(100);
                                                setFreq(1500);
                                                delay(100);
                                                setFreq(1900);
                                                delay(100);
                                                setFreq(1500);
                                                delay(100);
                                                setFreq(2300);
                                                delay(100);
                                                setFreq(1500);
                                                delay(100);
                                                setFreq(2300);
                                                delay(100);
                                                setFreq(1500);
                                                delay(100);
                                        }
                                        //-- VIS CODE --
                                        //VIS CODE for Scottie1 is B0111100 (DECIMAL 60)
                                        setFreq(1900);
                                        delay(300);
                                        setFreq(1200); //BREAK
                                        delay(10);
                                        setFreq(1900);
                                        delay(300);
                                        setFreq(1200); //START BIT
                                        delay(30);
                                        setFreq(1300); //BIT 0, 1 (LSB FIRST)
                                        delay(60);
                                        setFreq(1100); //BIT 2, 3, 4, 5
                                        delay(120);
                                        setFreq(1300); //BIT 6
                                        delay(30);
                                        setFreq(1300); //EVEN PARITY
                                        delay(30);
                                        setFreq(1200); //STOP BIT
                                        delay(30);
                                        //-- VIS DONE --
                                        
                                        //Starting sync
                                        syncTime = micros();
                                        
                                        for(i = 0; i < 320; i++)
                                        {
                                                lineBufR[i] = myFile1.read();
                                                lineBufG[i] = myFile1.read();
                                                lineBufB[i] = myFile1.read();
                                        }
                                        
                                        while(micros() - syncTime < 9000 - 10){}
                                        
                                        //Separator pulse
                                        setFreq(1500);
                                        syncTime = micros();
                                        
                                        line = 0;
                                        head = false;
                                }
                                
                                while(micros() - syncTime < 1500 - 10){}
                                
                                //Green scan
                                ti = 0; gbrCnt = 0; sSq = 3;
                                while(sSq == 3){};
                                
                                //Separator pulse
                                setFreq(1500);
                                while(micros() - syncTime < 1500 - 10){}
                                
                                //Blue scan
                                ti = 0; gbrCnt = 1; sSq = 3;
                                while(sSq == 3){};
                                
                                //Evacuate
                                for(i = 0; i < 320; i++)
                                {
                                        lineBufE[i] = lineBufR[i];
                                }
                                
                                //Read next line
                                if(line != 255)
                                {
                                        for(i = 0; i < 320; i++)
                                        {
                                                lineBufR[i] = myFile1.read();
                                                lineBufG[i] = myFile1.read();
                                                lineBufB[i] = myFile1.read();
                                        }
                                }
                                //Serial.println(micros() - syncTime); //Cheak reading time
                                /*
                                If horizontal sync is always unstable, uncomment the line above, and check reading time in the Serial window.
                                If the maximum reading time exceeds 9000uS, then please consider changing your micro SD card to a faster one.
                                */
                                
                                //Sync pulse
                                while(micros() - syncTime < 9000 - 10){}
                                
                                //Sync porch
                                setFreq(1500);
                                syncTime = micros();
                                while(micros() - syncTime < 1500 - 10){}
                                
                                //Red scan
                                ti = 0; gbrCnt = 2; sSq = 3;
                                while(sSq == 3){};
                                
                                line++;
                                if(line == 256)
                                {
                                        Serial.println("PICTURE SENT") ;
                                        Serial.println(sTimes);
                                        Serial.println();
                                        setFreq(2);
                                        sSq = 0;
                                        
                                        if(sTimes > 0)
                                        {
                                                sTimes--;
                                                
                                        }
                                        else
                                        {
                                                for(i = 0; i < 30; i++) // ***** INTERVAL (IF NECESSARY) *****
                                                {
                                                        delay(1000);
                                                }
                                        }
                                }
                                else
                                {
                                        //Separator pulse
                                        setFreq(1500);
                                        syncTime = micros();
                                        sSq = 2;
                                }
                        }
                }
                else
                {
                        Serial.println("CAN'T OPEN FILE");
                        err = true;
                }
                myFile1.close();
                
                if(err == true)
                {
                        digitalWrite(31, HIGH);
                        while(1);
                }
        }
}

