/*
  Fields to represent state of lamp.
*/
#define ON 1
#define OFF 0

/*
  Field to represent number of lamps on breadboard.
*/
#define LAMPS_MAX_COUNT 8

/*
  Field to control brightness changing.
*/
#define BRGHT_MIN 5
#define BRGHT_MAX 255
#define BRGHT_STEP 50

/*
  Fields for shift register to control all lamps.
  On breadboard, from left to right: Data->Latch->Clock.
*/
#define LATCHPIN 8
#define DATAPIN 11
#define CLOCKPIN 12

/*
  Field to control brightness.
*/
#define BRGHT_PIN 3

/*
  Class to represent lamp structure.
*/
class Lamp
{
  public:
    /*
      Class constructor.
      Sets indexes, global values, current values and shiftRegister id.
    */
    Lamp(int pinR_in, int pinG_in, int pinB_in,
         int valueR_in, int valueG_in, int valueB_in,
         int shiftRegister_in)
    {
      this->indexR = pinR_in;
      this->indexG = pinG_in;
      this->indexB = pinB_in;
      this->valueR = valueR_in;
      this->valueG = valueG_in;
      this->valueB = valueB_in;
      this->shiftRegister = shiftRegister_in;
    }

    /*
      {0,1,2,3,4,5,6,7}
      Index means input to shift register.
    */
    int indexR;
    int indexG;
    int indexB;

    /*
      {1,2,4,8,16,32,64,128}
      Value means bit representation. Depends on shift register.
      We need this to write certain byte to shift register data.
    */
    int valueR;
    int valueG;
    int valueB;

    /*
      1,2,3 - first, second or third register.
      4 - first and second registers.
      5 - second and third registers.
    */
    int shiftRegister;

    /*
      Lamp state - ON (1) or OFF (0).
    */
    int state = OFF;

    /*
      0 - red; 1 - green; 2 - blue.
    */
    int curColor = 0;
};

Lamp *lampSet[LAMPS_MAX_COUNT] = {
  // First shift registor.
  new Lamp(0, 1, 2, 1, 2, 4, 1),
  new Lamp(3, 4, 5, 8, 16, 32, 1),
  // First & second shift registor.
  new Lamp(6, 7, 0, 64, 128, 1, 4),
  // Second shift registor.
  new Lamp(1, 2, 3, 2, 4, 8, 2),
  new Lamp(4, 5, 6, 16, 32, 64, 2),
  // Second & third shift registors.
  new Lamp(7, 0, 1, 128, 1, 2, 5),
  // Third shift registor.
  new Lamp(2, 3, 4, 4, 8, 16, 3),
  new Lamp(5, 6, 7, 32, 64, 128, 3)
};

byte curBrightness = BRGHT_MAX;

/*
   Current lamps state.
*/
int srdata1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int srdata2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int srdata3[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup()
{
  pinMode(DATAPIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(BRGHT_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  char incomingByte = Serial.read();
  switch (incomingByte)
  {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      lamp_switchLamp(lampSet[(incomingByte - '0') - 1]);
      break;
    case 'q':
      lamp_changeColor(lampSet[0]);
      break;
    case 'w':
      lamp_changeColor(lampSet[1]);
      break;
    case 'e':
      lamp_changeColor(lampSet[2]);
      break;
    case 'r':
      lamp_changeColor(lampSet[3]);
      break;
    case 't':
      lamp_changeColor(lampSet[4]);
      break;
    case 'y':
      lamp_changeColor(lampSet[5]);
      break;
    case 'u':
      lamp_changeColor(lampSet[6]);
      break;
    case 'i':
      lamp_changeColor(lampSet[7]);
      break;
    case '-':
    case '+':
      lamp_changeBrightness(incomingByte);
      break;
    case 'a':
      for (int i = 0; i < LAMPS_MAX_COUNT; i++)
      {
        lamp_changeColor(lampSet[i]);
      }
      break;
    case 's':
      for (int i = 0; i < LAMPS_MAX_COUNT; i++)
      {
        lamp_switchLamp(lampSet[i]);
      }
      break;
    case 'z':
      for(int i = 0; i < 3; i++)
      {
        pingPong(); 
      }
      break;
  }
  lamp_updateLamps(srDataSum(srdata1),
                   srDataSum(srdata2),
                   srDataSum(srdata3)
                  );
}

void lamp_switchLamp(Lamp *curLamp)
{
  // If state was ON, turn lamp off.
  if (curLamp->state == ON)
  {
    switch (curLamp->shiftRegister)
    {
      case 1:
        srdata1[curLamp->indexR] = 0;
        srdata1[curLamp->indexG] = 0;
        srdata1[curLamp->indexB] = 0;
        break;
      case 2:
        srdata2[curLamp->indexR] = 0;
        srdata2[curLamp->indexG] = 0;
        srdata2[curLamp->indexB] = 0;
        break;
      case 3:
        srdata3[curLamp->indexR] = 0;
        srdata3[curLamp->indexG] = 0;
        srdata3[curLamp->indexB] = 0;
        break;
      case 4:
        srdata1[curLamp->indexR] = 0;
        srdata1[curLamp->indexG] = 0;
        srdata2[curLamp->indexB] = 0;
        break;
      case 5:
        srdata2[curLamp->indexR] = 0;
        srdata3[curLamp->indexG] = 0;
        srdata3[curLamp->indexB] = 0;
        break;
    }
  }
  // If state was OFF, turn lamp ON!
  else
  {
    switch (curLamp->curColor)
    {
      // If current color was red:
      case 0:
        switch (curLamp->shiftRegister)
        {
          case 1:
            srdata1[curLamp->indexR] = curLamp->valueR;
            srdata1[curLamp->indexG] = 0;
            srdata1[curLamp->indexB] = 0;
            break;
          case 2:
            srdata2[curLamp->indexR] = curLamp->valueR;
            srdata2[curLamp->indexG] = 0;
            srdata2[curLamp->indexB] = 0;
            break;
          case 3:
            srdata3[curLamp->indexR] = curLamp->valueR;
            srdata3[curLamp->indexG] = 0;
            srdata3[curLamp->indexB] = 0;
            break;
          case 4:
            srdata1[curLamp->indexR] = curLamp->valueR;
            srdata1[curLamp->indexG] = 0;
            srdata2[curLamp->indexB] = 0;
            break;
          case 5:
            srdata2[curLamp->indexR] = curLamp->valueR;
            srdata3[curLamp->indexG] = 0;
            srdata3[curLamp->indexB] = 0;
            break;
        }
        break;
      // If current color was green:
      case 1:
        switch (curLamp->shiftRegister)
        {
          case 1:
            srdata1[curLamp->indexR] = 0;
            srdata1[curLamp->indexG] = curLamp->valueG;
            srdata1[curLamp->indexB] = 0;
            break;
          case 2:
            srdata2[curLamp->indexR] = 0;
            srdata2[curLamp->indexG] = curLamp->valueG;
            srdata2[curLamp->indexB] = 0;
            break;
          case 3:
            srdata3[curLamp->indexR] = 0;
            srdata3[curLamp->indexG] = curLamp->valueG;
            srdata3[curLamp->indexB] = 0;
            break;
          case 4:
            srdata1[curLamp->indexR] = 0;
            srdata1[curLamp->indexG] = curLamp->valueG;
            srdata2[curLamp->indexB] = 0;
            break;
          case 5:
            srdata2[curLamp->indexR] = 0;
            srdata3[curLamp->indexG] = curLamp->valueG;
            srdata3[curLamp->indexB] = 0;
            break;
        }
        break;
      // If curColor was blue.
      case 2:
        switch (curLamp->shiftRegister)
        {
          case 1:
            srdata1[curLamp->indexR] = 0;
            srdata1[curLamp->indexG] = 0;
            srdata1[curLamp->indexB] = curLamp->valueB;
            break;
          case 2:
            srdata2[curLamp->indexR] = 0;
            srdata2[curLamp->indexG] = 0;
            srdata2[curLamp->indexB] = curLamp->valueB;
            break;
          case 3:
            srdata3[curLamp->indexR] = 0;
            srdata3[curLamp->indexG] = 0;
            srdata3[curLamp->indexB] = curLamp->valueB;
            break;
          case 4:
            srdata1[curLamp->indexR] = 0;
            srdata1[curLamp->indexG] = 0;
            srdata2[curLamp->indexB] = curLamp->valueB;
            break;
          case 5:
            srdata2[curLamp->indexR] = 0;
            srdata3[curLamp->indexG] = 0;
            srdata3[curLamp->indexB] = curLamp->valueB;
            break;
        }
        break;
    }
  }
  curLamp->state = (curLamp->state == ON) ? OFF : ON;
}

void lamp_changeBrightness(char incChar)
{
  if (incChar == '-')
  {
    if (curBrightness == BRGHT_MIN)
      return;
    curBrightness -=  BRGHT_STEP;
  }
  else
  {
    if (curBrightness == BRGHT_MAX)
      return;
    curBrightness += BRGHT_STEP;
  }
}

void lamp_changeColor(Lamp *curLamp)
{
  if (curLamp->state == OFF)
    return;
  switch (curLamp->curColor)
  {
    // If current color was red.
    case 0:
      curLamp->curColor = 1;
      switch (curLamp->shiftRegister)
      {
        case 1:
          srdata1[curLamp->indexR] = 0;
          srdata1[curLamp->indexG] = curLamp->valueG;
          break;
        case 2:
          srdata2[curLamp->indexR] = 0;
          srdata2[curLamp->indexG] = curLamp->valueG;
          break;
        case 3:
          srdata3[curLamp->indexR] = 0;
          srdata3[curLamp->indexG] = curLamp->valueG;
          break;
        case 4:
          srdata1[curLamp->indexR] = 0;
          srdata1[curLamp->indexG] = curLamp->valueG;
          break;
        case 5:
          srdata2[curLamp->indexR] = 0;
          srdata3[curLamp->indexG] = curLamp->valueG;
          break;
      }
      break;
    // If current color was green.
    case 1:
      curLamp->curColor = 2;
      switch (curLamp->shiftRegister)
      {
        case 1:
          srdata1[curLamp->indexG] = 0;
          srdata1[curLamp->indexB] = curLamp->valueB;
          break;
        case 2:
          srdata2[curLamp->indexG] = 0;
          srdata2[curLamp->indexB] = curLamp->valueB;
          break;
        case 3:
          srdata3[curLamp->indexG] = 0;
          srdata3[curLamp->indexB] = curLamp->valueB;
          break;
        case 4:
          srdata1[curLamp->indexG] = 0;
          srdata2[curLamp->indexB] = curLamp->valueB;
          break;
        case 5:
          srdata3[curLamp->indexG] = 0;
          srdata3[curLamp->indexB] = curLamp->valueB;
          break;
      }
      break;
    // If current color was blue.
    case 2:
      curLamp->curColor = 0;
      switch (curLamp->shiftRegister)
      {
        case 1:
          srdata1[curLamp->indexB] = 0;
          srdata1[curLamp->indexR] = curLamp->valueR;
          break;
        case 2:
          srdata2[curLamp->indexB] = 0;
          srdata2[curLamp->indexR] = curLamp->valueR;
          break;
        case 3:
          srdata3[curLamp->indexB] = 0;
          srdata3[curLamp->indexR] = curLamp->valueR;
          break;
        case 4:
          srdata2[curLamp->indexB] = 0;
          srdata1[curLamp->indexR] = curLamp->valueR;
          break;
        case 5:
          srdata3[curLamp->indexB] = 0;
          srdata2[curLamp->indexR] = curLamp->valueR;
          break;
      }
      break;
  }
}

void lamp_updateLamps(int srdata1_sum, int srdata2_sum, int srdata3_sum)
{
  // Update data in shift register.
  digitalWrite(LATCHPIN, LOW);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, srdata3_sum);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, srdata2_sum);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, srdata1_sum);
  digitalWrite(LATCHPIN, HIGH);
  // Update lampSet brightness.
  analogWrite(BRGHT_PIN, 255 - curBrightness);
}

int srDataSum(int *srData)
{
  int out = 0;
  for (int i = 0; i < 8; i++)
  {
    out += srData[i];
  }
  return (out);
}

void pingPong()
{
  int redSR[] = {1, 2, 4, 8, 16, 32, 64, 128};

  // RED RED RED
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, redSR[i]);
    digitalWrite(LATCHPIN, HIGH);
    delay(50);
  }
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, redSR[i]);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    digitalWrite(LATCHPIN, HIGH);
    delay(50);
  }
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, redSR[i]);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    digitalWrite(LATCHPIN, HIGH);
    delay(50);
  }
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, redSR[7 - i]);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    digitalWrite(LATCHPIN, HIGH);
    delay(50);
  }
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, redSR[7 - i]);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    digitalWrite(LATCHPIN, HIGH);
    delay(50);
  }
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(LATCHPIN, LOW);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0);
    shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, redSR[7 - i]);
    digitalWrite(LATCHPIN, HIGH);
    delay(50);
  }
}