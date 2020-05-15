#include<avr/io.h>
#define F_CPU 16000000UL
#include<util/delay.h>
#include<avr/interrupt.h>

#define OC0A 0

#define CTC_MODE  6
#define FAST_PWM  7
#define PHASE_CORRECTED 8

#define PRESCALER1 9
#define PRESCALER8 10
#define PRESCALER64 11
#define PRESCALER256 12
#define PRESCALER1024 13

#define PC_1  14

uint8_t receive_array[6] = {0,0,0,0,0,0};
int i = 0;

typedef struct
{
  uint8_t softPin;
  uint8_t dutyCycle;
  uint8_t inversion;
  uint16_t frequency;
}softwareConfig;

volatile softwareConfig PPS;

typedef struct
{
  uint8_t hardPin;
  uint8_t dutyCycle;
  uint8_t PWMMode;
  uint8_t inversion;
  uint8_t prescaler;
}hardwareConfig;

volatile hardwareConfig PPH;

void setup() 
{
  Serial.begin(9600);
  DDRC |= (1 << PORTC1);
  DDRD |= (1 << PORTD3) | (1 << PORTD5) | (1 << PORTD6);
  DDRB |= (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB5);

  PPH.hardPin = 1;
  PPH.dutyCycle = 80;
  PPH.PWMMode =  FAST_PWM;
  PPH.inversion = 0;
  PPH.prescaler = PRESCALER1;
  
  PPS.softPin = PC_1;
  PPS.dutyCycle = 35;
  PPS.inversion = 0;
  PPS.frequency = 5000;
  
  PWM0init();
  softwarePWM1init();
}

void loop() 
{
  if (i == 6) 
  {
    i = 0;
    PORTB ^= (1 << PORTB5);
    if (receive_array[0] == 1)
    {
      setPWMHardware();
    }
    else
    {
      setPWMSoftware();
    }
  }
}
void serialEvent() 
{
  while (Serial.available())
  {
    receive_array[i] = (uint8_t)Serial.read();
  }
  i++;
}


void setPWMSoftware()
{
  getSoftwareConfig();
  softwarePWM1init();
}

void setPWMHardware()
{
  getHardwareConfig();
  PWM0init();
}

void getSoftwareConfig()
{
  PPS.softPin = receive_array[1];
  PPS.dutyCycle = receive_array[2];
  PPS.inversion = receive_array[3];
  *(((uint8_t*)&PPS.frequency) + 1) = receive_array[4];
  *((uint8_t*)&PPS.frequency) = receive_array[5];
}

void getHardwareConfig()
{
  PPH.hardPin = receive_array[1];
  PPH.PWMMode = receive_array[2];
  PPH.inversion = receive_array[3];
  PPH.dutyCycle = receive_array[4];
  PPH.prescaler = receive_array[5];
}

void PWM0init()
{
  if (PPH.inversion == 1)
  {
    TCCR0A |= (1 << COM0A1) | (1 << COM0A0);
    TCCR0A |= (1 << COM0B1) | (1 << COM0B0);  
  }
  else
  {
    TCCR0A |= (1 << COM0A1);
    TCCR0A &= ~(1 << COM0B0);
    TCCR0A |= (1 << COM0B1);
    TCCR0A &= ~(1 << COM0A0);
  }
  //Pre-scaler
  if (PPH.prescaler == PRESCALER1)
  {
    TCCR0B |= (1 << CS00);
    TCCR0B &= ~(1 << CS01);
    TCCR0B &= ~(1 << CS02);
  }
  else if (PPH.prescaler == PRESCALER8)
  {
    TCCR0B |= (1 << CS01);
    TCCR0B &= ~(1 << CS02);
    TCCR0B &= ~(1 << CS00);
  }
  else if (PPH.prescaler == PRESCALER64)
  {
    TCCR0B |= (1 << CS01) | (1 << CS00);
    TCCR0B &= ~(1 << CS02);
  }
  else if (PPH.prescaler == PRESCALER256)
  {
    TCCR0B |= (1 << CS02);
    TCCR0B &= ~(1 << CS00);
    TCCR0B &= ~(1 << CS01);
  }
  else if (PPH.prescaler == PRESCALER1024)
  {
    TCCR0B |= (1 << CS02) | (1 << CS00);
    TCCR0B &= ~(1 << CS01);
  }
  
  //Duty cycle
  OCR0A = (PPH.dutyCycle/100.0)*256.0;
  OCR0B = (PPH.dutyCycle/100.0)*256.0;
  
  // MODE
  if (PPH.PWMMode == CTC_MODE)
  {
    TCCR0A &= ~(1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0A |= (1 << COM0B0);
    TCCR0A |= (1 << COM0A0);
    TCCR0A &= ~(1 << COM0A1);
    TCCR0A &= ~(1 << COM0B1); 
  }
  if (PPH.PWMMode == FAST_PWM)
  {
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    TCCR0B &= ~(1 << WGM02);
  }
  else if (PPH.PWMMode == PHASE_CORRECTED)
  {
    TCCR0A &= ~(1 << WGM01);
    TCCR0A |= (1 << WGM00);
    TCCR0B &= ~(1 << WGM02);
  }
}

void softwarePWM1init()
{
  OCR1B = F_CPU/(float)(PPS.frequency);
  OCR1A = (PPS.dutyCycle/100.0)*(uint16_t)OCR1B;

  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B);
  //Set interrupt on compare match

  TCCR1B |= (1 << CS10); //PRESCALER1
  sei();
}

ISR (TIMER1_COMPA_vect)
{
  if (PPS.inversion == 1)
  {
    PORTC |= (1 << PORTC1);
  }
  else
  {
    PORTC &= ~(1 << PORTC1);
  }
};

ISR (TIMER1_COMPB_vect)
{
  if (PPS.inversion == 1)
  {
    PORTC &= ~(1 << PORTC1);
  }
  else
  {
    PORTC |= (1 << PORTC1);
  }
  TCNT1 = 0;
};
