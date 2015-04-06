#include <TimerOne.h>

#define  TRIAC_CTRL_PIN_0  (4)
#define  TRIAC_CTRL_PIN_1  (5)
#define  TRIAC_CTRL_PIN_2  (6)
#define  TRIAC_CTRL_PIN_3  (7)
#define  INT_PIN          (2)

#define  DEBUG_PIN_0      (8)
#define  DEBUG_PIN_1      (9)

typedef struct {
  int ctrlPin;
  unsigned int waitTim;
  unsigned int nowTim;
  bool isRun;
} T_TRIAC_CTRLPARAM;

T_TRIAC_CTRLPARAM param[4];

#define  PARAM_SIZE    (sizeof(param) / sizeof(T_TRIAC_CTRLPARAM))

#define  MAX_TIM       (500)
#define  MAX_COUNT     (20)    // 1 / 50Hz / 2 / 500us

#define  BYTE2TIM(b)  (MAX_COUNT - ((b) * MAX_COUNT / 0xFF))

char buff[100];
unsigned int index;

typedef struct {
  uint8_t devID;
  uint8_t cmdID;
  uint8_t usrCmdID;
} T_SERIAL_CMD_HEADER;

byte chex2dec(char c)
{
  if (('0' <= c) && (c <= '9')) {
    return c - '0';
    
  } else if (('a' <= c) && (c <= 'f')) {
    return c - 'a' + 10;
    
  } else if (('A' <= c) && (c <= 'F')) {
    return c - 'A' + 10;
  }
  
  return 0;
}

byte hex2dec(char buff[])
{
  return (chex2dec(buff[0]) << 4) + chex2dec(buff[1]);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(DEBUG_PIN_0, OUTPUT);
  digitalWrite(DEBUG_PIN_0, LOW);
  pinMode(DEBUG_PIN_1, OUTPUT);
  digitalWrite(DEBUG_PIN_1, LOW);
  
  param[0].ctrlPin = TRIAC_CTRL_PIN_0;
  param[0].waitTim = 0;
  param[0].nowTim = 0;
  param[0].isRun = false;
  param[1].ctrlPin = TRIAC_CTRL_PIN_1;
  param[1].waitTim = 0;
  param[1].nowTim = 0;
  param[1].isRun = false;
  param[2].ctrlPin = TRIAC_CTRL_PIN_2;
  param[2].waitTim = 0;
  param[2].nowTim = 0;
  param[2].isRun = false;
  param[3].ctrlPin = TRIAC_CTRL_PIN_3;
  param[3].waitTim = 0;
  param[3].nowTim = 0;
  param[3].isRun = false;

  for (int i = 0; i < PARAM_SIZE; i++) {
    pinMode(param[i].ctrlPin, OUTPUT);
    digitalWrite(param[i].ctrlPin, LOW);
  }

  pinMode(INT_PIN, INPUT);

  attachInterrupt(0, intZeroCross, RISING);

  Timer1.initialize(MAX_TIM);
  Timer1.attachInterrupt(tick500us);
  Timer1.start();
  
  memset(buff, 0, sizeof(buff));
  index = 0;
  Serial.begin(38400);
}

void loop() {
  digitalWrite(DEBUG_PIN_1, HIGH);
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    char c = Serial.read();
    
    if (c != '\r') {
      if (c == '\n') {
        // Do
        if (index >= 5) {
          T_SERIAL_CMD_HEADER header;
          header.devID = hex2dec(&buff[1]);
          header.cmdID = hex2dec(&buff[3]);
          
          if (index >= 10) {
            header.usrCmdID = hex2dec(&buff[5]);
    
            if (header.cmdID == 1) {
              if (header.usrCmdID == 1) {
                byte data01 = hex2dec(&buff[7]); 
                byte data02 = hex2dec(&buff[9]);
                byte data03 = hex2dec(&buff[11]);
                byte data04 = hex2dec(&buff[13]);
                            
                param[0].waitTim = BYTE2TIM(data01);
                param[1].waitTim = BYTE2TIM(data02);
                param[2].waitTim = BYTE2TIM(data03);
                param[3].waitTim = BYTE2TIM(data04);
                
                Serial.println(data01, HEX);
              }
            }
          }
        }
        
        // ~~~~~
        index = 0;
        
      } else {
        buff[index] = c;
        index++;
        if (index >= sizeof(buff)) {
          index = 0;
        }
      }
    }
  }
  digitalWrite(DEBUG_PIN_1, LOW);
}

void intZeroCross(void)
{
  for (int i = 0; i < PARAM_SIZE; i++) {
    param[i].nowTim = 0;
    param[i].isRun = true;
    digitalWrite(param[i].ctrlPin, LOW);
  }
}

void tick500us(void)
{
  digitalWrite(DEBUG_PIN_0, HIGH);

  for (int i = 0; i < PARAM_SIZE; i++) {
    if (param[i].isRun) {
      param[i].nowTim++;
      if (param[i].nowTim >= param[i].waitTim) {
        param[i].isRun = false;
        digitalWrite(param[i].ctrlPin, HIGH);
      }
    }
  }
  Timer1.restart();
  
  digitalWrite(DEBUG_PIN_0, LOW);
}


