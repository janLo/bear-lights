/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// Based on Arduino Beat Detector By Damian Peckett 2015 (License: Public
// Domain.)

#include <Arduino.h>

//#define SERIAL_OUT
#define SIMPLE_RGB

// Our Global Sample Rate, 5000hz
#define SAMPLEPERIODUS 200

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#ifdef SIMPLE_RGB
uint8_t r[] = {0x19, 0x21, 0x29, 0x32, 0x3b, 0x44, 0x4b, 0x54, 0x5d, 0x65, 0x6e,
               0x77, 0x88, 0x9d, 0xb2, 0xc7, 0xda, 0xde, 0xe2, 0xe6, 0xeb, 0xed,
               0xef, 0xf1, 0xf2, 0xf4, 0xf6, 0xf7, 0xf9, 0xfa, 0xfc, 0xff};
uint8_t g[] = {0xa9, 0xac, 0xae, 0xb1, 0xb3, 0xb5, 0xb7, 0xbb, 0xbd, 0xc0, 0xc2,
               0xc4, 0xc9, 0xcf, 0xd6, 0xdc, 0xde, 0xc5, 0xad, 0x95, 0x7c, 0x6b,
               0x62, 0x59, 0x4f, 0x45, 0x3b, 0x31, 0x28, 0x1e, 0x15, 0x0b};
uint8_t b[] = {0x15, 0x15, 0x14, 0x13, 0x13, 0x12, 0x12, 0x11, 0x10, 0x10, 0x0f,
               0x0f, 0x0e, 0x0c, 0x0b, 0x0a, 0x0a, 0x19, 0x26, 0x34, 0x44, 0x4d,
               0x52, 0x58, 0x5e, 0x63, 0x69, 0x6e, 0x74, 0x7a, 0x7f, 0x85};
#endif

void setup() {
  // Set ADC to 77khz, max for 10bit
  sbi(ADCSRA, ADPS2);
  cbi(ADCSRA, ADPS1);
  cbi(ADCSRA, ADPS0);

  // The pin with the LED
  pinMode(2, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);

#ifdef SERIAL_OUT
  Serial.begin(115200);
  Serial.println("Start");
#endif
}

// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
  static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
  xv[0] = xv[1];
  xv[1] = xv[2];
  xv[2] = sample / 9.1f;
  yv[0] = yv[1];
  yv[1] = yv[2];
  yv[2] = (xv[2] - xv[0]) + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
  return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
float envelopeFilter(float sample) { // 10hz low pass
  static float xv[2] = {0, 0}, yv[2] = {0, 0};
  xv[0] = xv[1];
  xv[1] = sample / 160.f;
  yv[0] = yv[1];
  yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
  return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
float beatFilter(float sample) {
  static float xv[3] = {0, 0, 0}, yv[3] = {0, 0, 0};
  xv[0] = xv[1];
  xv[1] = xv[2];
  xv[2] = sample / 7.015f;
  yv[0] = yv[1];
  yv[1] = yv[2];
  yv[2] = (xv[2] - xv[0]) + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
  return yv[2];
}

void loop() {
  unsigned long time = micros(); // Used to track rate
  float sample, value, envelope, beat, thresh;
  unsigned char i;

#ifdef SIMPLE_RGB
  float max = 0.0f;
  int16_t rr, gg, bb;

#endif

  for (i = 0;; ++i) {
    // Read ADC and center so +-512
    sample = (float)analogRead(0) - 503.f;

    // Filter only bass component
    value = bassFilter(sample);

    // Take signal amplitude and filter
    if (value < 0)
      value = -value;
    envelope = envelopeFilter(value);

    // Every 200 samples (25hz) filter the envelope
    if (i == 200) {
      // Filter out repeating bass sounds 100 - 180bpm
      beat = beatFilter(envelope);

      // Threshold it based on potentiometer on AN1
      thresh = 0.02f * (float)analogRead(1);

      // If we are above threshold, light up LED
      if (beat > thresh) {
        digitalWrite(2, HIGH);

#ifdef SIMPLE_RGB
        max = max(max, beat);
        uint8_t val = round(((beat - thresh) / (max - thresh)) * 32);
#endif

#ifdef SIMPLE_RGB
        max = max * 0.9f;

        rr = min(255, r[val] + 30);
        gg = max(0, g[val] - 40);
        bb = min(255, b[val] + 20);
#endif

      } else {
        digitalWrite(2, LOW);

#ifdef SIMPLE_RGB
        rr = max(0, rr - 20);
        gg = max(0, gg - 20);
        bb = max(0, bb - 20);
#endif
      }

#ifdef SIMPLE_RGB
      analogWrite(9, rr);
      analogWrite(10, gg);
      analogWrite(11, bb);
#endif

      // Reset sample counter
      i = 0;
    }

    // Consume excess clock cycles, to keep at 5000 hz
    for (unsigned long up = time + SAMPLEPERIODUS; time > 20 && time < up;
         time = micros())
      ;
  }
}
