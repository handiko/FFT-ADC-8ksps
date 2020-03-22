#include <Arduino.h>

#include <fix_fft.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SAMPLES 128
#define M 7
#define ADC_CHANNEL 0

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define OLED_ADDRESS 0x3C

int adc_counter = 0;
char vReal[SAMPLES], vImag[SAMPLES];

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//------------------------------------------------//
//                DECLARATIONS                    //
//------------------------------------------------//

void init_io_led();
void toggle_led();
void init_adc();
void startADC();
void init_timer();
void init_OLED(uint8_t oled_address);

ISR(ADC_vect)
{
  TIFR1 = (1 << OCF1B);

  vReal[adc_counter] = ADCH;
  vImag[adc_counter] = 0;

  adc_counter++;

  toggle_led();
}

void setup()
{
  // put your setup code here, to run once:
  init_adc();
  init_io_led();
  init_timer();

  init_OLED(OLED_ADDRESS);

  startADC();

  sei();
}

void loop()
{
  if (adc_counter == SAMPLES)
  {
    /*fix_fft(vReal, vImag, M, false);

    display.clearDisplay();

    //double dat;

    for (int i = 0; i < SAMPLES; i++)
    {
      display.drawPixel(i, sqrt(pow(vReal[i], 2) + pow(vImag[i], 2)), WHITE);
    }

    display.display();

    // delay(10);

    adc_counter = 0;*/

    display.clearDisplay();

    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.print(int(vReal[0]));
    display.print(" ");
    display.print(int(vImag[0]));

    display.display();

    delay(1);

    adc_counter = 0;
  }
}

// ---------------------------- DEFINITIONS -------------------//
//                                                             //
// ------------------------------------------------------------//

void init_io_led()
{
  DDRB |= (1 << PB5);
}

void toggle_led()
{
  PORTB ^= (1 << PB5);
}

void init_adc()
{
  // REFS0 : select AVCC as reference
  // ADLAR : left adjust (8 bit mode by reading the ADCH)
  ADMUX |= (1 << REFS0) | (1 << ADLAR);

  // ADPS0 = 1 ADPS1 = 1 : set prescaller = 3 --> 2^3 = 8 (divide by 8)
  // ADEN : enable the ADC
  // ADATE : enable auto trigger, source is set by ADTS
  // ADIE : ADC interrupt at conversion is complete
  ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN) | (1 << ADATE) | (1 << ADIE);

  // ADTS2 = 1 ADTS0 = 1 : select auto trigger source, Timer1 compare match B
  ADCSRB |= (1 << ADTS2) | (1 << ADTS0);
}

void startADC()
{
  ADCSRA |= (1 << ADSC);
}

void init_timer()
{
  // TIMER 1 for interrupt frequency 1 Hz:
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B

  TCNT1 = 0; // initialize counter value to 0

  // set compare match register for 1 Hz increments
  OCR1A = 15999; // = 16000000 / (256 * 1) - 1 (must be <65536)

  // Set CS12, CS11 and CS10 bits for prescaler
  // WGM12 = 1 turn on CTC mode
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10) | (1 << WGM12);
}

void init_OLED(uint8_t oled_address)
{
  display.begin(SSD1306_SWITCHCAPVCC, oled_address);
  display.clearDisplay();
  display.display();
}