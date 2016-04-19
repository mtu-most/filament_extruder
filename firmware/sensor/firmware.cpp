// Firmware for the attiny in the sensor.
/*
   Communication protocol:
   Sensor is bus master; host is slave.
   SS is not used.  Instead, MISO is pulled low by host to initiate reset.
   After reset, sensor sends number of used pixels and enters main loop.
   During main loop with DEBUG_MODE set, every pixel is sent to the host.
   During main loop without DEBUG_MODE set, the position of the shadow is sent for every frame, followed by 0x53.
   If the host does not receive 0x53, it requests a reset.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

// Hardware:
// IC connections:
// 1: reset			b5	i
// 2: sensor ao		adc3	b3	i
// 3: sensor clk		b4	o
// 4: gnd
// 5: mosi			b0	o
// 6: miso/sensor si		b1	o
// 7: sck			b2	o
// 8: vcc

#define HWPIXELS 768
#define SKIP 81	// On both sides.
#define EACHBITS 5
#define EACHMASK ((1 << EACHBITS) - 1)
#define VALID_LIMIT 100	// Maximum value for min_ao to still be valid.
#define PIXELS ((HWPIXELS - 2 * SKIP) >> EACHBITS)
#define DELAY 100
#define SHORT_DELAY 5

#define CLK_PIN 4
#define SI_PIN 1
#define DO_PIN 0
#define SCK_PIN 2

#define ddrb 0x15	// output for bits 0, (1), 2, 4.
#define adcsra 0x80	// Enable ADC, prescaler 2. (should be 8 for full range.)

static inline void delay(uint32_t time) {
	for (uint32_t i = 0; i < time; ++i)
		asm volatile ("nop");
}

static void send_data(uint8_t pos) {
	for (uint8_t i = 0; i < 8; ++i) {
		if ((pos >> i) & 1) {
			PORTB = 1 << DO_PIN;
			PORTB = (1 << DO_PIN) | (1 << SCK_PIN);
			delay(SHORT_DELAY);
			PORTB = 1 << DO_PIN;
			delay(SHORT_DELAY);
		}
		else {
			PORTB = 0;
			PORTB = 1 << SCK_PIN;
			delay(SHORT_DELAY);
			PORTB = 0;
			delay(SHORT_DELAY);
		}
	}
}

// While changing si, all other pins are always 0.
static void set_si() {
	DDRB = ddrb;	// SI input.
	PORTB = 1 << SI_PIN; // with pullup.
}

static void pulse() {
	int portb = PORTB;
	PORTB = portb | (1 << CLK_PIN);
	PORTB = portb;
}

static void clear_si() {
	PORTB = 0;	// No pullup.
	DDRB = ddrb | (1 << SI_PIN);	// output.
}

volatile int8_t state = 0;
volatile int8_t current = 0xff;
volatile int16_t sum;

// States:
// 0: normal measument.
// 1: normal; seen at least one measurement.
// bit 6: first reading after sync; ignore.
// bit 7: wait for sync.

ISR(TIMER1_COMPA_vect) {
	if (state & 0x80)
		return;
	set_si();
	pulse();
	if (!(PINB & (1 << SI_PIN))) {
		state |= 0xc0;
		// Synchronize; wait for the pin to return to high.
		while (!(PINB & (1 << SI_PIN))) {}
		// Finish reading.
		pulse();
		clear_si();
		for (int i = 0; i < HWPIXELS; ++i)
			pulse();
		// Send sensor length.
		send_data(PIXELS & 0xff);
		state &= ~0x80;	// Leave 0x40 set, so the first reading is ignored.
		TIFR = 1 << OCF1A;
		return;
	}
	clear_si();
	int idx = 0, min_idx = -1;
	int min_ao = 0x100;
	int i = 0;
	sum = 0;
	for (; i < SKIP; ++i)
		pulse();
	for (; i < HWPIXELS - SKIP; ++i) {
		if ((i & EACHMASK) == 0) {
			ADCSRA = adcsra | (1 << ADSC);
			while (ADCSRA & (1 << ADSC)) {}
			int ao = ADCH & 0xff;
			sum += ao;
#ifndef DEBUG_MODE
			if (ao < min_ao) {
				min_ao = ao;
				min_idx = idx;
			}
#else
			send_data(ao);
#endif
			idx += 1;
		}
		pulse();
	}
	for (; i < HWPIXELS; ++i)
		pulse();
	if (state == 0 && (true || min_ao < VALID_LIMIT)) {
		current = min_idx;
		state = 1;
	}
	else
		state &= ~0x40;
}

static inline void set_state(uint8_t s) {
	cli();
	state = (state & ~0x2f) | s;
	sei();
}

int main() {
	// Set up the pins.
	PORTB = 0;
	DDRB = ddrb;
	// Set up the ADC.
	ADCSRA = adcsra;
	ADCSRB = 0x00;
	ADMUX = 0x23;
	DIDR0 = 1 << ADC3D;
	// Set up the clock.
	TCCR1 = 0xcf;	// PWM, 2 full overflows per second.
	OCR1A = 1;
	OCR1C = DELAY;	// More overflows per second.
	GTCCR = 0;
	TIFR = 1 << OCF1A;
	TIMSK = 1 << OCIE1A;
	sei();
	while(1) {
#ifndef DEBUG_MODE
		set_state(0);
		while (state != 1) {}
		send_data(current);
		send_data(0x53);
#endif
	}
}
