#include <Wire.h>
#include <elapsedMillis.h>

#define LCDADDRESS 0x3F
#define ENVIROADDRESS 0x77

#define BUTTONPIN 2

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x80
#define LCD_NOBACKLIGHT 0x00

  //  DB7, DB6, DB5, DB4, BL, EN, RW, ID
#define ENABLE 0x4  // Enable bit
#define COMMAND 0x1


elapsedMillis timeElapsed;
 


void sendByte(byte byteData)
{
  Wire.beginTransmission(LCDADDRESS);
  Wire.write((int)byteData);
  Wire.endTransmission(); 
}
void pulseEnable(byte byteData)
{
  sendByte(byteData | ENABLE);
  delayMicroseconds(1);
  sendByte(byteData & ~ENABLE);
  delayMicroseconds(1);
}
void sendNybble(bool command, byte nybble)
{
  if (!command)
  {
    pulseEnable((int)(nybble<<4) | COMMAND);
  }
  else
  {
    pulseEnable((int)(nybble<<4) & ~COMMAND);
  }
}

void initLCD()
{
  delayMicroseconds(45000);
  sendNybble(1, 0x03);
  delayMicroseconds(45000);
  sendNybble(1, 0x03);
  delayMicroseconds(1000);
  sendNybble(1, 0x03);
  delayMicroseconds(45000);
  sendNybble(1, 0x02);
  delayMicroseconds(1000);
}

void LCD_Send(bool command, byte data)
{
  sendNybble(command, (data & 0xf0)>>4);
  sendNybble(command, data & 0x0f);
}
void LCD_Repeat(bool command, byte data, int times)
{
  for(int _ = 0; _ < times; _++)
  {
    LCD_Send(command, data);
  }
}

void LCDPrint(String text)
{
  for(int i = 0; i < text.length(); i++)
  {
    char chr = text.charAt(i);
    LCD_Send(0, chr);
  }
}
void LCD_Clear()
{
  LCD_Send(1, LCD_CLEARDISPLAY);
  delayMicroseconds(10000);
}
void LCD_Home()
{
  LCD_Send(1, LCD_RETURNHOME);
  delayMicroseconds(10000);
}
void LCD_Tab(int line, int column)
{  
  switch (line%4)
  {
    case 0:LCD_Send(1, LCD_SETDDRAMADDR | (column));break;
    case 1:LCD_Send(1, LCD_SETDDRAMADDR | (column + 64));break;
    case 2:LCD_Send(1, LCD_SETDDRAMADDR | (column + 20));break;
    case 3:LCD_Send(1, LCD_SETDDRAMADDR | (column + 84));break;
  }
}
void defChar(byte chr, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5, byte data6, byte data7)
{
  LCD_Send(1, LCD_SETCGRAMADDR | (chr * 8));
  LCD_Send(0, data0);
  LCD_Send(0, data1);
  LCD_Send(0, data2);
  LCD_Send(0, data3);
  LCD_Send(0, data4);
  LCD_Send(0, data5);
  LCD_Send(0, data6);
  LCD_Send(0, data7);
  
}


class SSprite
{
  private:
  int xPos;
  int yPos;
  int xCol;
  byte chr;
  byte character[8] = {};

  public:
  SSprite(byte cStart, byte a0, byte a1, byte a2, byte a3, byte a4, byte a5, byte a6, byte a7): chr(cStart), xPos(-99), yPos(-99), xCol(-99)
  {
    character[0] = a0;
    character[1] = a1;
    character[2] = a2;
    character[3] = a3;
    character[4] = a4;
    character[5] = a5;
    character[6] = a6;
    character[7] = a7;
  }
  
  void moveTo(int line, int x)
  {
    int col = (x + 12) / 6 - 2;
    int index = ((x%6) + 6) % 6;
    
    if (col != xCol || line != yPos)
    {
      draw(xCol, yPos, 0x20, 0x20);
      draw(col, line, chr, chr + 1);
    }
    xCol = col;
    yPos = line;
    xPos = x;

    UpdateChr(index);
    
  }
  

  void draw(int column, int row, byte left, byte right)
  {
    if (column < -1) return;
    
    if (column == -1)
    {
      LCD_Tab(row, 0);
      LCD_Send(0, right);
    }
    else if (column < 19)
    {
      LCD_Tab(row, column);
      LCD_Send(0, left);
      LCD_Send(0, right);  
    }
    else if (column == 19)
    {
      LCD_Tab(row, 19);
      LCD_Send(0, left);
    }
    else
    {
      return;
    }
  }

  void UpdateChr(int index)
  {
    LCD_Send(1, LCD_SETCGRAMADDR | (chr*8));
    for(int i = 0; i < 8; i++) LCD_Send(0, ((int)character[i] >> (2 + index)) & 0x1F);
    if (index == 5)
    {
      for(int i = 0; i < 8; i++) LCD_Send(0, ((int)character[i] >> 1) & 0x1F);
    }
    else
    {
      for(int i = 0; i < 8; i++) LCD_Send(0, ((int)character[i] << (4 - index)) & 0x1F);
    }
  }
  
  void mask(byte chrRow, int index)
  {
    int chrs = ((int)chrRow);
    int left  = (chrs >> (2 + index)) & 0b11111;
    int right = (chrs << (4 - index)) & 0b11111;    
  }

  /*  How it works!
 012345678901234567890123    x col ix   >>2+ix              << 4-ix
  BCDEF H0000 ..... .....    0  0   0    A.BCDEF.GH                
  ABCDE GH000 ..... .....    1  0   1     .ABCDE.FGH
  0ABCD FGH00 ..... .....    2  0   2     .0ABCD.EFGH
  00ABC EFGH0 ..... .....    3  0   3     .00ABC.DEFGH
  000AB DEFGH ..... .....    4  0   4     .000AB.CDEFGH
  0000A CDEFG ..... .....    5  0   5     .0000A.BCDEFGH
  ..... BCDEF H0000 .....    6  1   0
  ..... ABCDE GH000 .....    7  1   1
  ..... 0ABCD FGH00 .....    8  1   2
  ..... 00ABC EFGH0 .....    9  1   3
  ..... 000AB DEFGH .....   10  1   4
  ..... 0000A CDEFG .....   11  1   5
  ..... ..... BCDEF H0000   12  2   0
  ..... ..... ABCDE GH000   13  2   1
*/


};



class RSprite
{
  private:
  byte chr;
  int character[8] = {};
  int index;
  
  public:
  RSprite(byte cStart,  int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7): chr(cStart), index(0)
  {
    character[0] = a0 & 0xFFF;
    character[1] = a1 & 0xFFF;
    character[2] = a2 & 0xFFF;
    character[3] = a3 & 0xFFF;
    character[4] = a4 & 0xFFF;
    character[5] = a5 & 0xFFF;
    character[6] = a6 & 0xFFF;
    character[7] = a7 & 0xFFF;
  }
  void draw(int line)
  {
    LCD_Tab(line, 0);
    for(int i = 0; i < 10; i++)
    {
      LCD_Send(0,chr);
      LCD_Send(0,chr + 1);
    }
  }
  void stepIndex(int shift)
  {
    index += shift;
    index %= 12;
    LCD_Send(1, LCD_SETCGRAMADDR | (chr*8));

    if (index == 0)
    {
      for(int i = 0; i < 8; i++) LCD_Send(0, (byte)((character[i] >> 13) | (character[i] >> 1) & 0x1F));
    }
    else
    {
      for(int i = 0; i < 8; i++) LCD_Send(0, (byte)((character[i] >> (13 - index)) | (character[i] << (index - 1)) & 0x1F));
    }

    if (index > 7)
    {
      for(int i = 0; i < 8; i++) LCD_Send(0, (byte)((character[i] >> (19 - index)) | (character[i] << (index - 7)) & 0x1F));
    }
    else
    {
      for(int i = 0; i < 8; i++) LCD_Send(0, (byte)((character[i] >> (19 - index)) | (character[i] >> (7 - index)) & 0x1F));
    } 
  }
};

class VSprite
{
  private:
  byte chr;
  int character[11 * 2] = {};
  int column;
  int y;
  int yRow;
  
  public:
  VSprite(byte cStart, byte a0, byte a1, byte a2, byte a3, byte a4, byte a5, byte a6, byte a7, byte a8, byte a9, byte a10): chr(cStart), column(-99), y(-99), yRow(-99)
  {
    character[0] = a0;
    character[1] = a1;
    character[2] = a2;
    character[3] = a3;
    character[4] = a4;
    character[5] = a5;
    character[6] = a6;
    character[7] = a7;
    character[8] = a8;
    character[9] = a9;
    character[10] = a10;
  }

  void moveTo(int newY, int newCol)
  {
    int newRow = (newY + 18) / 9 - 2;
    int newIndex = ((newY % 9) + 9) % 9;

    if (column != newCol || yRow != newRow)
    {
      draw(column, yRow, 0x20, 0x20);
      draw(newCol, newRow, chr, chr + 1);
    }
    column = newCol;
    y = newY;
    yRow = newRow;

    UpdateChr(newIndex);
  }
  void forceDraw()
  {
    draw(column, yRow, chr, chr + 1);
  }
  
  void draw(int column, int row, byte top, byte bottom)
  {
    if (column < 0) return;
    if (column > 19) return;
    if (row < -1) return;

    if (row == -1)
    {
      LCD_Tab(0, column);
      LCD_Send(0, bottom); 
    }
    else if (row < 3)
    {
      LCD_Tab(row, column);
      LCD_Send(0, top);
      LCD_Tab(row + 1, column);
      LCD_Send(0, bottom);
    }
    else if (row == 3)
    {
      LCD_Tab(3, column);
      LCD_Send(0, top);
    }
  }

  void UpdateChr(int newIndex)
  {
    LCD_Send(1, LCD_SETCGRAMADDR | (chr*8));
    
    int ptr = 0;
    if (newIndex == 0) ptr++;
    int i = 0;
    for(; i < newIndex - 1; i++) {LCD_Send(0, 0);}
    for(; i < 8; i++) {LCD_Send(0, character[ptr++]);}
    ptr++;
    for(; ptr < 11;) {LCD_Send(0, character[ptr++]);}
    
    for(int i = 0; i < 7 - newIndex; i++) {LCD_Send(0, 0);}
    
  }

/*  index     0   1   2   3   4   5   6   7   8     9
    ign   .   l0
    chrA  0   l1  l0  .   .   .   .   .   .   .     .
          1   l2  l1  l0  .   .   .   .   .   .     .
          2   l3  l2  l1  l0  .   .   .   .   .     .
          3   l4  l3  l2  l1  l0  .   .   .   .     .
          4   l5  l4  l3  l2  l1  l0  .   .   .     .
          5   l6  l5  l4  l3  l2  l1  l0  .   .     .
          6   l7  l6  l5  l4  l3  l2  l1  l0  .     .
          7   l8  l7  l6  l5  l4  l3  l2  l1  l0    .
    ign   .   l9  l8  l7  l6  l5  l4  l3  l2  l1    l0
    chrB  8   lA  l9  l8  l7  l6  l5  l4  l3  l2    l1
          9   .   lA  l9  l8  l7  l6  l5  l4  l3    l2  
          A   .   .   lA  l9  l8  l7  l6  l5  l4    l3  
          B   .   .   .   lA  l9  l8  l7  l6  l5    l4  
          C   .   .   .   .   lA  l9  l8  l7  l6    l5  
          D   .   .   .   .   .   lA  l9  l8  l7    l6  
          E   .   .   .   .   .   .   lA  l9  l8    l7  
          F   .   .   .   .   .   .   .   lA  l9    l8  
    ign   .   .   .   .   .   .   .   .   .   lA    l9
    ign   .   .   .   .   .   .   .   .   .   .     lA

    index 0 1
    wrld
    start 1 0
    stop  8
    start -
    stop  -
*/
};


SSprite cactus(4, 
  0b0000011,
  0b0100011,
  0b0110011,
  0b0011110,
  0b0011000,
  0b1111000,
  0b0011000,
  0b0011000);

RSprite gameFloor(2,
  0b000111111000,
  0b001100001100,
  0b111000000111,
  0b000000000000,
  0b111111000000,
  0b100001100001,
  0b000000111111,
  0b000000000000);


VSprite dragon(6,
0b01111,
0b01100,
0b01111,
0b01100,
0b01110,
0b01100,
0b01110,
0b01011,
0b01001,
0b10001,
0b00000);






const int fRCycle = 75;

float cactusDX = -10.0;

float cactusX = 120.0;

const float dragonZ = 17.0;
const float dragonG = 22.0;
const float dragonJ = -23.0;

float dragonY = dragonZ;
float dragonDY = 0.0;

int buttonState = LOW;

int score = 10;

bool colliding = false;
bool hasHit = false; 

void PrintScore()
{
  String st = String(score);
  LCD_Tab(0, 14);
  LCD_Repeat(0, 0x20, 6 - st.length());
  LCDPrint(st);
}

void startGame()
{
  LCD_Send(1, LCD_DISPLAYCONTROL | LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF);
  
  cactusDX = -15.0;
  cactusX = 120.0;
  dragonY = dragonZ;
  dragonDY = 0.0;
  score = 10;
  colliding = false;
  hasHit = false;
  
  LCD_Clear();
  
  gameFloor.draw(3);
  
  LCD_Tab(1, 6);
  LCDPrint("PRESS START");

  delay(1000);
  LCD_Send(1, LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);

  while (digitalRead(BUTTONPIN) == LOW) {delay(50);gameFloor.stepIndex(1);}
  while (digitalRead(BUTTONPIN) == HIGH) {delay(50);gameFloor.stepIndex(1);}
          
  LCD_Tab(1, 6);
  LCDPrint("           ");

  PrintScore();
  dragon.forceDraw();
  
  timeElapsed = 0;
}

void setup() {
  Serial.begin (115200);
  while(!Serial);

  Wire.begin();
  initLCD();
  LCD_Send(1, LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x10DOTS);
  LCD_Send(1, LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
  
  LCD_Clear();

  pinMode(BUTTONPIN, INPUT);
  

  startGame();
}

void loop() {
  int dCycle = timeElapsed;  
  delay(fRCycle - timeElapsed%fRCycle);

  Serial.println(dCycle);
  
  float deltaTime = 0.001 * (float)timeElapsed;
  timeElapsed = 0;

  // Physics
  dragonY += dragonDY * deltaTime;
  dragonDY += dragonG * deltaTime;
  if (dragonY >= dragonZ)
  {
    dragonDY = 0.0;
    dragonY = dragonZ;
  }

  float u = (int)cactusX;
  cactusX += cactusDX * deltaTime;
  int stp = u - (int)cactusX;
  
  if (cactusX < -20.0)
  {
    cactusX = 120.0;
  }
  cactusDX -= 0.1 * deltaTime;

  // Check controls
  int newButtonState = digitalRead(BUTTONPIN);
  bool buttonDown = false;
  if (newButtonState == HIGH && buttonState == LOW)
  {
    dragonDY = dragonJ;
  }
  buttonState = newButtonState; 

  gameFloor.stepIndex(stp);
  
  dragon.moveTo((int)dragonY, 1);
  cactus.moveTo(2, (int)cactusX);

  if (cactusX <= 13 && cactusX > -2)
  {
    if (!colliding)
    {
      colliding = true;
      hasHit = false;
      
      LCD_Tab(0, 3);
      LCDPrint("     ");
    }
    
    if (dragonY > dragonZ - 1.0)
    {
      hasHit = true;  
    }

  }
  else
  {
    if (colliding)
    {
      colliding = false;
      
      LCD_Tab(0, 3);
      if (hasHit)
      {
        LCDPrint("Hit  ");
        score -= 200;
        if (score < 0)
        {

          LCD_Tab(1, 6);
          LCDPrint(" Game Over "); 
          delay(3000);
          
          startGame();
          return;
        }
      }
      else
      {
        LCDPrint("Clear");
        score += 50;  
      }

      
  PrintScore();
    }

  }
}






