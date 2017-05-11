//#include <usb_serial.h>
#include <Adafruit_CircuitPlayground.h>
//#include <Adafruit_NeoPixel.h>


typedef void (*callback_function)(void);

enum  direction { FORWARD, REVERSE };
enum  pattern { NONE, FULL, INCREMENT, FADE, FLASH };

union ULONG_T
{
  unsigned long value;
  byte bytes[4];
};
union FLOAT_T
{
  float value;
  byte bytes[4];
};
struct color
{
  int red;
  int green;
  int blue;
};

struct pixel
{
  color Color;
  color Color1;
  color Color2;
  bool fade=false;
  uint16_t Index;
  uint16_t TotalSteps;
  direction Direction; 
  unsigned long Interval;
  unsigned long lastUpdate;
  bool on=false;
};

pattern Pattern = FADE;
unsigned long Interval;
unsigned long lastUpdate;
pixel Pixel[10];
void (*OnComplete)();

int brightness=0;
float PatternLastUpdate=0;
int brightnessStep=1;
int colorCounter=0;
int pixelCounter=0;
int patternCounter=0;
bool pixelFlash = false;
uint32_t Cyan = 0x00FFFF;
uint32_t Black = 0x000000;
uint32_t White = 0xFFFFFF;
uint32_t Red = 0xFF0000;
uint32_t Green = 0x00FF00;
uint32_t Blue = 0x0000FF;
uint32_t Magenta = 0xFF00FF;
uint32_t Yellow = 0xFFFF00;
uint32_t colors[8];
byte pin = 0;
int wait;
FLOAT_T x;
FLOAT_T y;
FLOAT_T z;
FLOAT_T tempC;
FLOAT_T tempF;
FLOAT_T light;
FLOAT_T sound;
bool left;
bool right;
bool fadeComplete=false;
bool incrementComplete=false;
bool flashComplete=false;
bool slideSwitch;
void fade(int i,color color1, color color2, uint16_t steps, uint16_t interval, direction dir = FORWARD);

void setup() {
  // put your setup code here, to run once:
  CircuitPlayground.begin();
  Serial.begin(9600);
  Interval=50;
  lastUpdate=0;
  pixelFlash=true;
  Pattern=NONE;
  colors[0]=Red;
  colors[1]=Green;
  colors[2]=Blue;
  colors[3]=Cyan;
  colors[4]=Yellow;
  colors[5]=Magenta;
  colors[6]=White;
  colors[7]=Black;
  //Serial.println("Hello");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.print("Hello");
  Update(doUsb);//doSerial);
  checkPattern();
  readUsb();
  if(Serial.available()>9)
  {
    readSerial();
  }

  fadeUpdate();

  DrawAllPixels();
}

void DrawAllPixels()
{
  for (int i=0;i<10;i++)
    CircuitPlayground.setPixelColor(i,Pixel[i].Color.red,Pixel[i].Color.green,Pixel[i].Color.blue);
}

color Color(uint32_t c)
{
  color temp;
  temp.red = c >> 16 &0xff;
  temp.green = c >> 8 & 0xff;
  temp.blue = c & 0xff;

  return temp;
}

void readUsb()
{
  byte bytebuffer[65];
  uint8_t bytesRecieved = RawHID.recv(bytebuffer,0);
  if(bytesRecieved>0)
  {
    //usb_serial_putchar(buffer[0]);
    Serial.print(bytebuffer[0]);
    /*byte packet[64];
    for (int i=0;i<64;i++)
    {
      packet[i]=buffer[i];
    }*/
      /*for (int i=0;i<10;i++)
        ClearPixel(i);
      pixelFlash=true;
      Pattern=FLASH;
      PatternLastUpdate=0;
      colorCounter=0;*/
    byte command = bytebuffer[0];
    readPacket(command,bytebuffer);
  }
}
void readSerial()
{
  byte packet [13];
  for(int i=0;i<13;i++)
    packet[i]=Serial.read();
  byte command = packet[0];
  readPacket(command,packet);

  
}

void writeUsb()
{
  byte buffer[64];
  buffer[0] = x.bytes[0];
  buffer[1] = x.bytes[1];
  buffer[2] = x.bytes[2];
  buffer[3] = x.bytes[3];
  buffer[4] = y.bytes[0];
  buffer[5] = y.bytes[1];
  buffer[6] = x.bytes[2];
  buffer[7] = x.bytes[3];
  buffer[8] = z.bytes[0];
  buffer[9] = z.bytes[1];
  buffer[10] = z.bytes[2];
  buffer[11] = z.bytes[3];
  buffer[12] = tempC.bytes[0];
  buffer[13] = tempC.bytes[1];
  buffer[14] = tempC.bytes[2];
  buffer[15] = tempC.bytes[3];
  buffer[16] = tempF.bytes[0];
  buffer[17] = tempF.bytes[1];
  buffer[18] = tempF.bytes[2];
  buffer[19] = tempF.bytes[3];
  buffer[20] = light.bytes[0];
  buffer[21] = light.bytes[1];
  buffer[22] = light.bytes[2];
  buffer[23] = light.bytes[3];
  buffer[24] = sound.bytes[0];
  buffer[25] = sound.bytes[1];
  buffer[26] = sound.bytes[2];
  buffer[27] = sound.bytes[3];
  buffer[28] = left;
  buffer[29] = right;
  buffer[30] = slideSwitch;
  RawHID.send(buffer,0);
}
void writeSerial()
{
  Serial.print(x.value);
  Serial.print(",");
  Serial.print(y.value);
  Serial.print(",");
  Serial.print(z.value);
  Serial.print(",");
  Serial.print(tempC.value);
  Serial.print(",");
  Serial.print(tempF.value);
  Serial.print(",");
  Serial.print(light.value);
  Serial.print(",");
  Serial.print(sound.value);
  Serial.print(",");
  Serial.print(left);
  Serial.print(",");
  Serial.print(right);
  Serial.print(",");
  Serial.println(slideSwitch);
}

void readPacket(byte command, byte packet [])
{
    switch(command)
  {
    case 0xAA:
    {
      byte pin,r,g,b;
      if (Pattern!=NONE)
      {
        Pattern=NONE;
        for (int i=0;i<10;i++)
          ClearPixel(i);
      }
      pin = packet[1];
      Pixel[pin].Color.red = packet[2];
      Pixel[pin].Color.green = packet[3];
      Pixel[pin].Color.blue = packet[4];
      Pixel[pin].fade = false;
      Pixel[pin].on = true;
      //for (int i=0;i<9;i++)
      //Serial.read();

        
      //turnOnLed(pin,r,g,b);
      Pattern = NONE;

      break;
    }
    case 0x01:
    {
      if (Pattern!=NONE)
      {
        for (int i=0;i<10;i++)
        ClearPixel(i);
      }
      pin = packet[1];
      Pixel[pin].Color1.red = packet[2];
      Pixel[pin].Color1.green = packet[3];
      Pixel[pin].Color1.blue = packet[4];
      Pixel[pin].Color2.red = packet[5];
      Pixel[pin].Color2.green = packet[6];
      Pixel[pin].Color2.blue = packet[7];
      ULONG_T waitArray;
      for (int i=0;i<4;i++)
      {
        waitArray.bytes[i]=packet[8+i];
      }
      //Serial.read();
      wait = waitArray.value/255;
      
      fade(pin,Pixel[pin].Color1,Pixel[pin].Color2,255,wait);
      Pixel[pin].fade = true;
      Pixel[pin].on = true;
      Pattern = NONE;
      break;
    }
    case 0x02:
    {
      for(int i=0;i<10;i++)
      {
        ClearPixel(i);
      }
      Pattern = NONE;
      break;
    }
    case 0x03:
    {
      for (int i=0;i<10;i++)
        ClearPixel(i);
      Pattern=FULL;
      PatternLastUpdate=0;
      colorCounter=0;
      pixelCounter=0;
      incrementComplete=false;
      fadeComplete=false;
      flashComplete=false;
      break;
    }
    case 0x04:
    {
      for (int i=0;i<10;i++)
        ClearPixel(i);
      Pattern=INCREMENT;
      PatternLastUpdate=0;
      colorCounter=0;
      pixelCounter=0;
      break;
    }
    case 0x05:
    {
      for (int i=0;i<10;i++)
        ClearPixel(i);
      Pattern=FADE;
      PatternLastUpdate=0;
      colorCounter=0;
      for(int i=0;i<10;i++)
      {
        Pixel[i].Color = Color(Cyan);
      }
      break;
    }
    case 0x06:
    {
      for (int i=0;i<10;i++)
        ClearPixel(i);
      pixelFlash=true;
      Pattern=FLASH;
      PatternLastUpdate=0;
      colorCounter=0;
      break;
    }
    default:
      break;
  }
}

void readSensors()
{
  x.value = CircuitPlayground.motionX();
  y.value = CircuitPlayground.motionY();
  z.value = CircuitPlayground.motionZ();
  tempC.value = CircuitPlayground.temperature();
  tempF.value = CircuitPlayground.temperatureF(); 
  light.value = CircuitPlayground.lightSensor();
  sound.value = CircuitPlayground.soundSensor();
  left = CircuitPlayground.leftButton();
  right = CircuitPlayground.rightButton();
  slideSwitch = CircuitPlayground.slideSwitch();
}

void ClearPixel(int i)
{
  Pixel[i].Color.red=0;
  Pixel[i].Color.green=0;
  Pixel[i].Color.blue=0;
  
  Pixel[i].Color1.red=0;
  Pixel[i].Color1.green=0;
  Pixel[i].Color1.blue=0;
  
  Pixel[i].Color2.red=0;
  Pixel[i].Color2.green=0;
  Pixel[i].Color2.blue=0;
  
  Pixel[i].Index = 0;
  Pixel[i].TotalSteps = 0;
  Pixel[i].Direction = FORWARD; 
  Pixel[i].Interval = 0;
  Pixel[i].lastUpdate = 0;
  Pixel[i].fade=false;
  Pixel[i].on=false;
}

void checkPattern()
{
  switch(Pattern)
  {
    case NONE:
      break;
    case FULL:
    {
      FullPattern();
      break;
    }
    case INCREMENT:
    {
      incrementUpdate(3);
      break;
    }
    case FADE:
    {
      //fadeUpdate();
      BrightnessUpdate();
      break;
    }
    case FLASH:
    {
      Flash();
      break;
    }
    default:
      break;
  }
  if(Pattern!=FULL)
    patternCounter=0;
}
void BrightnessUpdate()
{
  
    if((millis() - PatternLastUpdate) > 1000/30) // time to update
    {
      PatternLastUpdate = millis();
      brightness+=brightnessStep;  
    }

    if(brightness>30)
    {
      brightness=30;
      brightnessStep = brightnessStep*-1;
    }
    else if (brightness<0)
    {
      brightness=0;
      brightnessStep = brightnessStep*-1;
      fadeComplete=true;
    }
    CircuitPlayground.setBrightness(brightness);
}
void FullPattern()
{
  if(incrementComplete==false)
  {
    incrementUpdate(1);
    if(incrementComplete==true)
    {
      colorCounter=0;
      pixelCounter=0;
      PatternLastUpdate=0;
      patternCounter++;
      for(int i=0;i<10;i++)
      {
        Pixel[i].Color=Color(Cyan);
      }
    }
  }
  else if (fadeComplete==false)
  {
    BrightnessUpdate(); 
    if(fadeComplete==true)
    {
      colorCounter=0;
      pixelCounter=0;
      PatternLastUpdate=0;
      for(int i=0;i<10;i++)
        ClearPixel(i);
      CircuitPlayground.setBrightness(30);
      pixelFlash=true;
    }
  }
  else if (flashComplete==false)
  {
    Flash();
    if(flashComplete==true)
    {
      colorCounter=0;
      pixelCounter=0;
      PatternLastUpdate=0;
    }
  }

  if(incrementComplete==true && fadeComplete==true && flashComplete==true)
  {;
    PatternLastUpdate=0;
    pixelCounter=0;
    incrementComplete=false;
    fadeComplete=false;
    flashComplete=false;
    if (patternCounter>2)
      patternCounter=0;
    colorCounter=patternCounter;
  }

}
void incrementUpdate(int colorCap)
{
    if((millis() - PatternLastUpdate) > 250) // time to update
    {
      PatternLastUpdate = millis();
      Pixel[pixelCounter].Color=Color(colors[colorCounter]);
      pixelCounter++;  
    }

    if(pixelCounter>9)
    {
      pixelCounter=0;
      colorCounter++;
    }
    if(colorCounter>=colorCap+patternCounter)
    {
      colorCounter=0;
      incrementComplete=true;
    }
}

void Flash()
{
    if((millis() - PatternLastUpdate) > 500) // time to update
    {
      PatternLastUpdate = millis();
      if(pixelFlash==true)
      {
        for(int i=0;i<10;i++)
          Pixel[i].Color=Color(colors[colorCounter]);
        pixelFlash=false;
        
      }  
      
      else if(pixelFlash==false)
      {
        for(int i=0;i<10;i++)
          Pixel[i].Color=Color(Black);
        pixelFlash=true;
        colorCounter++;
      }  
    }
    if(colorCounter>=7)
    {
      colorCounter=0;
      flashComplete=true;
    }
  
}
void Increment(int i)
{
    if (Pixel[i].Direction == FORWARD)
    {
       Pixel[i].Index++;
       if (Pixel[i].Index >= Pixel[i].TotalSteps)
        {
            Pixel[i].Index = Pixel[i].TotalSteps;
            Pixel[i].Direction = REVERSE;
            /*if (OnComplete != NULL)
            {
                OnComplete(); // call the comlpetion callback
            }*/
        }
    }
    else // Direction == REVERSE
    {
        --Pixel[i].Index;
        if (Pixel[i].Index <= 0)
        {
            Pixel[i].Index = 0;
            Pixel[i].Direction = FORWARD;
            /*if (OnComplete != NULL)
            {
                OnComplete(); // call the comlpetion callback
            }*/
        }
    }
}

void fade(int i, color color1, color color2, uint16_t steps, uint16_t interval, direction dir)
{
    //ActivePattern = FADE;
    Pixel[i].Interval = interval;
    Pixel[i].TotalSteps = steps;
    Pixel[i].Color1 = color1;
    Pixel[i].Color2 = color2;
    Pixel[i].Index = 0;
    Pixel[i].fade=true;
    Pixel[i].lastUpdate=0;
    //Pixel[i].Direction = dir;
}

void fadeUpdate()
{
  for(int i=0;i<10;i++)
  {
    if (Pixel[i].fade==true)
    {
      if((millis() - Pixel[i].lastUpdate) > Pixel[i].Interval) // time to update
      {
        Pixel[i].lastUpdate=millis();
        int temp = ((Pixel[i].Color1.red * (Pixel[i].TotalSteps - Pixel[i].Index)) + (Pixel[i].Color2.red * Pixel[i].Index)) / Pixel[i].TotalSteps;
        if(temp>255)
          Pixel[i].Color.red = 255;
        else
          Pixel[i].Color.red = temp;
  
        temp = ((Pixel[i].Color1.green * (Pixel[i].TotalSteps - Pixel[i].Index)) + (Pixel[i].Color2.green * Pixel[i].Index)) / Pixel[i].TotalSteps;
        if(temp>255)
          Pixel[i].Color.green = 255;
        else
          Pixel[i].Color.green = temp;
  
        temp = ((Pixel[i].Color1.blue * (Pixel[i].TotalSteps - Pixel[i].Index)) + (Pixel[i].Color2.blue * Pixel[i].Index)) / Pixel[i].TotalSteps;
        if(temp>255)
          Pixel[i].Color.blue = 255;
        else
          Pixel[i].Color.blue = temp;
          
        //CircuitPlayground.setPixelColor(i,red, green, blue);
        
        Increment(i);
      }
    }
  }
}

void Update(callback_function pFunc)
{
    if((millis() - lastUpdate) > Interval) // time to update
    {
        lastUpdate = millis();
        pFunc();  
    }
}

void doUsb()
{
  readSensors();
  writeUsb();
}
void doSerial()
{
  readSensors();
  writeSerial();
}

