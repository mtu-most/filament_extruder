// vim: set foldmethod=marker filetype=cpp :
// Copyright 2015 Michigan Technological University {{{
// Based upon work by Ian Johnson (http://www.thingiverse.com/thing:174383)
// Author: Jerry Anzalone and Bas Wijnen
// This design was developed as part of a project with
// the Michigan Tech Open Sustainability Technology Research Group
// http://www.appropedia.org/Category:MOST
// It is released under CC-BY-3.0
// If you do not have a copy of the license, it may be found at: http://creativecommons.org/licenses/by/3.0/legalcode
// }}}

/*{{{*********  CONTROL LOGIC  **********

Controls are a rotary encoder with push button and a 16x2 lcd display.

At startup, the system is in auto mode.

Pressing the button enters the main menu.  Menu structure:

Main
	Manual Winder Speed -> Button to exit menu.
	Temperature
		Set target
		Set PID constants/auto-calibrate
	Filament Guide
		Set left
		Set right
		Set # steps
	QC
		Set calibration constants
	Back


}}} */

#define dbg(msg, data) do { Serial.print(msg); Serial.println(data); } while (0)
//#define SERIAL_LCD	// Enable LCD output to serial port.
//#define DEBUG_SENSOR	// Sensor data to serial port.
//#define FILAWINDER	// Use filawinder board.

uint8_t checkButton(bool block);

// Includes. {{{
#include <EEPROM.h>
#include <Arduino.h>
//#include <Servo.h> 
#include "PID_v1.h"
#include <LcdMenu.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
// }}}

/* Filawinder pin markings: {{{
0	rx		serial		+
1	tx		serial		+
2	auto +		UI A/B		+
3	guide min +	UI B/A		+
4	sensor cal +	UI Select	+
5	winder FET	Winder FET	+
6	servo		Servo		+
7	hall (H side)	Winder hall	#
8	guide max +	buzzer		x
9	h8				-
10	spi/h8		spi		+
11	spi/h8		spi		+
12	spi/h8		spi		+
13	spi/h8		spi		+
A0	sensor(20k)	heater FET	+
A1	sensor(20k)	auger FET	+
A2	sensor(20k)	QC length hall	#
A3	sensor(20k)	QC diameter	#
A4	i2c		i2c sda		+
A5	i2c		i2c scl		+
A6	knob				-
A7	h6		temperature	+
+-------------+
|  a0  a1  a2 |
| gnd vcc  a3 |
+----     ----+

icsp: key towards avr.
}}} */

/* Conversion between pin change interrupt numbers, port+pin pairs and arduino pin numbers: {{{
pcint	avr	pin		used as
00	B0	8		-				auger FET (this pin and/or 7)
01	B1	9		oc1a: winder FET
02	B2	10	spi ss	oc1b: guide servo		Cannot be used; is SS.
03	B3	11		spi mosi: icsp
04	B4	12		spi miso: icsp
05	B5	13		spi sck: icsp
06	B6			xtal1
07	B7			xtal2

08	C0	A0		c0: QC distance
09	C1	A1		c1: hall effect sensor
10	C2	A2		a2: temperature input
11	C3	A3		c3: buzzer
12	C4	A4		i2c sda: display
13	C5	A5		i2c scl: display
14	C6			reset: icsp
15	n/a	n/a

16	D0	0		uart rxd: serial
17	D1	1		uart txd: serial
18	D2	2		pcint18: ui a
19	D3	3		pcint19: ui b
20	D4	4		pcint20: ui select
21	D5	5		oc0b: auger FET			guide servo
22	D6	6		oc0a: heater FET
23	D7	7		-				auger FET (this pin and/or 8)

		A6		-
		A7		QC: diameter

}}} */

/*{{{************ Hardware connections. ************/
#ifdef FILAWINDER
// {{{
	// UI.
#define PIN_A		3
#define PIN_B		2
#define PC_PORT_NAME PIND
#define PC_CONTROL_BIT	2
#define PC_PORT_VECT PCINT2_vect
#define PIN_A_MASK	0x08
#define PIN_B_MASK	0x04
#define PIN_BUTTON	4
#define PIN_BUTTON_MASK	0x10
	// Heater.
#define PIN_TEMP	A7
#define PIN_HEATER	A0
// Define this as ! to invert the signal.
#define INVERT_HEATER !
	// Winder.
#define PIN_WINDER	5
#define PIN_HALL	7
#define PIN_GUIDE 	6
	// Auger motor.
#define PIN_AUGER	A1
// Define this as ! to invert the signal.
#define INVERT_AUGER !
	// QC.
#define PIN_LEN		A2
#define PIN_DIA		A3

// Spare: 8, 9, A6
// }}}
#else
// {{{
	// UI.
#define PIN_A		2
#define PIN_B		3
#define PC_PORT_NAME PIND
#define PC_CONTROL_BIT	2
#define PC_PORT_VECT PCINT2_vect
#define PIN_A_MASK	0x04
#define PIN_B_MASK	0x08
#define PIN_BUTTON	4
#define PIN_BUTTON_MASK	0x10
	// Heater.
#define PIN_TEMP	A2
#define PIN_HEATER	5
// Define this as ! to invert the signal.
#define INVERT_HEATER
	// Winder.
#define PIN_WINDER	6
#define PIN_HALL	A1
#define PIN_GUIDE 	9
	// Auger motor.
#define PIN_AUGER	8
// Define this as ! to invert the signal.
#define INVERT_AUGER
	// QC.
#define PIN_LEN		A3
#define PIN_DIA		A7

// Spare: 7, A3, A6
// }}}
#endif
// Sensor is on SPI: 10, 11, 12, 13.
// 10,11,12,13: SPI
// A4,A5: I2C
// 0,1: UART
// }}}

/************ Globals. ******************/
// UI. {{{
static bool send_status = true;
#define EEPROM_MAGIC 0x5a	// Content of EEPROM[0] to signify valid contents.
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define debounce_time 10	// milliseconds.
#define knob_debounce_time 100	// milliseconds.
#define reset_time 1000	// time to hold button for lcd to reset in milliseconds.
static int *edit_num_target = NULL;
static volatile bool clicked;
static volatile int8_t delta;
static volatile unsigned long reset_start_time;
// }}}
// Heater. {{{
static uint8_t heater_phase, heater_duty;
static unsigned long last_temp_update;
static float heater_output;
static int raw_temp;
static uint8_t force_heater = 0;
static int T_KP = 10;	// %
static int T_KI = 0;	// %
static int T_KD = 0;	// %
// These are used to limit the value of the I buffer.
#define T_OP_MIN 0.
#define T_OP_MAX 100.
#define T_SAMPLE_TIME 1000  //ms
static double T_sp, T_ip, T_op = 50;
static PID T_PID(&T_ip, &T_op, &T_sp, T_KP, T_KI / 1000., T_KD, DIRECT);
#define INITIAL_TEMP_TARGET 170
#ifdef THERMOCOUPLE
#define TEMP_A 1
#define TEMP_B 0
#else
#define ADCBITS 10
#define R0 10e3
#define R1 INFINITY
#define Rc 100e3
#define Tc (25 + 273.15)
float beta = 4420;
#define K (exp(log(Rc) - beta / Tc))
#endif
// }}}
// Winder. {{{
static int hall_state = HIGH;
static int last_hall_state = HIGH;
// Set these values based on the servo you are using.
#define GUIDE_PERIOD (1. / 50)
#define GUIDE_MIN_TIME 1e-3
#define GUIDE_MAX_TIME 3e-3
// These values should never change.
#define PRESCALE 8
#define GUIDE_TICK_TIME (PRESCALE * 1. / F_CPU)
#define GUIDE_TICKS_PER_PERIOD (uint16_t(GUIDE_PERIOD / GUIDE_TICK_TIME + .5))
#define GUIDE_MIN_TICKS (uint16_t(GUIDE_MIN_TIME / GUIDE_TICK_TIME + .5))
#define GUIDE_MAX_TICKS (uint16_t(GUIDE_MAX_TIME / GUIDE_TICK_TIME + .5))
static double guide_min = -30;		// guide limiting positions.
static double guide_max = 30;
static int guide_direction = 1;		// direction  changes upon reaching min or max
static double guide_current;		// current guide position
static int guide_steps = 40;		// the number of increments for the guide to cover the range.
static volatile uint8_t sensor = 0;
static volatile uint8_t pixels;
static uint8_t last_pixel = 0;
#define CONST 2
// }}}
// Auger motor. {{{
#define MIN_TEMP 130
#define MAX_TEMP 300
static bool auger_on = false;
static int8_t force_winder = 0;	// -1 is auto, 0 is off, 1 is max.
// }}}
// QC. {{{
static float diameter = 0;
static int dia_slope = 10;	// This is divided by 10 on use.
static int dia_intercept = 0.;	// This is divided by 10 on use.
static int len_hall_state = HIGH;
static int last_len_hall_state = HIGH;
static unsigned total_length = 0;	// This is divided by 10 on use.
static int len_step = 355;	// This is divided by 10 on use.
static unsigned long last_report;	// For limiting QC output to serial.
// }}}

/*************** Functions. *****************/
// Winder. {{{
static volatile uint8_t sensor_state = 0;
static volatile uint8_t sensor_sync_phase = 1;
#ifdef DEBUG_SENSOR
#define SENSOR_DATA_LEN 20
static volatile uint8_t sensor_data[SENSOR_DATA_LEN];
static volatile uint8_t sensor_state_data[SENSOR_DATA_LEN];
static volatile uint8_t sensor_data_pos = 0;
#endif
ISR(SPI_STC_vect) { // {{{
	uint8_t data = SPDR;
#ifdef DEBUG_SENSOR
	if (sensor_data_pos < SENSOR_DATA_LEN) {
		sensor_data[sensor_data_pos] = data;
		sensor_state_data[sensor_data_pos++] = sensor_state;
	}
#endif
	if (sensor_state == 0) {
		pixels = data;
		sensor_state = 1;
	}
	else if (sensor_state == 1) {
		sensor = data;
		sensor_state = 2;
	}
	else {
		if (sensor_sync_phase == 0)
			sensor_sync_phase = data != 0x53;
		sensor_state = 1;
	}
} // }}}

static void sync_sensor() { // {{{
	static int8_t wait = 0;
	switch(sensor_sync_phase) {
	case 0:
		return;
	case 1:
		digitalWrite(PIN_HEATER, INVERT_HEATER(false));
		digitalWrite(PIN_AUGER, INVERT_AUGER(false));
		Serial.println("Syncing sensor");
		lcd.clear();
		lcd.print("Syncing sensor");
		pinMode(SCK, INPUT);
		sensor_sync_phase = 2;
		pixels = 0;
		wait = 10;
		// Fall through.
	case 2:
		if (pixels == 0) {
			wait -= 1;
			if (wait > 0)
				return;
			Serial.println("Sensor syncing");
			wait = 10;
			pinMode(MISO, OUTPUT);
			digitalWrite(MISO, LOW);
			SPCR = 0x00;
			delay(100);
#ifdef DEBUG_SENSOR
			//sensor_data_pos = 0;
#endif
			sensor_state = 0;
			SPCR = 0xe0;
			pinMode(MISO, INPUT);
			return;
		}
		// Sync completed.
		sensor_sync_phase = 0;
		Serial.print("Sensor synchronized: ");
		Serial.println(pixels);
	}
} // }}}

static uint8_t handle_winder() { // {{{
	sync_sensor();
	if (sensor_sync_phase)
		return 0;
#ifdef DEBUG_SENSOR // {{{
	if (true) {
		cli();
		uint8_t len = sensor_data_pos;
		Serial.print("sensor data:");
		for (uint8_t i = 0; i < len; ++i) {
			Serial.print(" ");
			Serial.print(sensor_data[i]);
		}
		Serial.print("\nsensor state:");
		for (uint8_t i = 0; i < len; ++i) {
			Serial.print(" ");
			Serial.print(sensor_state_data[i]);
		}
		Serial.print("\n");
		sensor_data_pos -= len;
		sei();
	}
#endif // }}}
	if (force_winder >= 0) {
		return force_winder * 255;
	}
	uint8_t pixel = sensor;
	if (pixel > 0 && pixel < pixels - 1)
		last_pixel = pixel;
	//dbg("sensor: ", pixel);
	float pos = ((last_pixel * 1.0 / pixels - .5) * 2 * CONST) / 2 + .5; // range 0-1
	//dbg("measure ", pos);
	if (pos <= 0)
		return 0;
	else if (pos >= 1)
		return 255;
	else
		return 255 * pos;
} // }}}

static void set_guide(double angle) { // {{{
	uint16_t value = (angle + 90) * (GUIDE_MAX_TICKS - GUIDE_MIN_TICKS) / 180 + GUIDE_MIN_TICKS;
	OCR1AH = value >> 8;
	OCR1AL = value & 0xff;
} // }}}

static void handle_guide() { // {{{
	if (edit_num_target) {
		return;
	}
	hall_state = digitalRead(PIN_HALL);
	if (hall_state == LOW && hall_state != last_hall_state) {
		last_hall_state = hall_state;
		// it just switched to high, calculate what to do with the guide
		// the calculation of position is entirely dependent upon the position of the filament in the guide
		// but the filament runs through a slot, so it could be anywhere.
		// provide means for adjusting increment on the fly.
		guide_current += guide_direction * 1.0 * (guide_max - guide_min) / guide_steps;
		if (guide_current >= guide_max) {
			guide_direction = -1;
			guide_current = guide_max;
		}
		if (guide_current <= guide_min) {
			guide_direction = 1;
			guide_current = guide_min;
		}
		set_guide(guide_current);
	}
	else
		last_hall_state = hall_state;
} // }}}
// }}}
// UI. {{{
extern uint8_t const PROGMEM chars[8][8] = {
	{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},	// heat off, auger off, auger not requested.
	{0x02, 0x07, 0x07, 0x07, 0x06, 0x00, 0x0c, 0x0c},	// heat off, auger off, auger requested. (ALARM!)
	//{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	// heat off, auger on, auger not requested (doesn't happen).
	{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},	// Instead use a copy of char 0, because it is treated as end of string.
	{0x00, 0x00, 0x00, 0x04, 0x04, 0x15, 0x0e, 0x04},	// heat off, auger on, auger requested.
	{0x11, 0x11, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},	// heat on, auger off, auger not requested.
	{0x11, 0x11, 0x1f, 0x1b, 0x1b, 0x0a, 0x11, 0x1b},	// heat on, auger off, auger requested.
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	// heat on, auger on, auger not requested (doesn't happen).
	{0x0e, 0x0e, 0x00, 0x04, 0x04, 0x15, 0x0e, 0x04}};	// heat on, auger on, auger requested.
static void init_lcd() { // {{{
	lcd.init();
	lcd.backlight();
	lcd.clear();
	for (uint8_t i = 0; i < 8; ++i) {
		uint8_t data[8];
		for (uint8_t b = 0; b < 8; ++b)
			data[b] = pgm_read_byte(&chars[i][b]);
		lcd.load_custom_character(i, data);
	}
	//lcd_reset = false;
} // }}}

ISR(PC_PORT_VECT) {	// {{{
	static bool a, b;
	uint8_t data = PC_PORT_NAME;
	bool new_a = data & PIN_A_MASK;
	bool new_b = data & PIN_B_MASK;
	bool new_select = !(data & PIN_BUTTON_MASK);
	if (!new_select) {
		reset_start_time = millis();
	}
	else if (millis() - reset_start_time > debounce_time)
		clicked = true;
	if (a != new_a) {
		delta += new_a ^ new_b ? 1 : -1;
	}
	if (b != new_b) {
		delta += new_a ^ new_b ? -1 : 1;
	}
	a = new_a;
	b = new_b;
} // }}}
uint8_t checkButton(bool block) {	// {{{
	static unsigned long idle_time;
	uint8_t ret = 0;
	cli();
	if (clicked) {
		ret |= BUTTON_SELECT;
		clicked = false;
	}
	if (delta >= 4) {
		ret |= BUTTON_DOWN;
		delta -= 4;
		idle_time = millis();
	}
	else if (delta <= -4) {
		ret |= BUTTON_UP;
		delta += 4;
		idle_time = millis();
	}
	else if (idle_time + knob_debounce_time > millis())
		delta = 0;
	sei();
	return ret;
} // }}}
static uint8_t edit_num(int &target, char const *msg, int min, int max, bool digit) {	// {{{
	if (edit_num_target != &target) {
		edit_num_target = &target;
		message(msg, "");
		lcd.setCursor(0, 1);
		if (digit) {
			if (target < 0)
				lcd.print("-");
			lcd.print(abs(target) / 10);
			lcd.print(".");
			lcd.print(abs(target) % 10);
		}
		else
			lcd.print(target);
		lcd.print("      ");
#ifdef SERIAL_LCD
		Serial.println(target);
#endif
	}
	uint8_t button = checkButton(false);
	if (button & BUTTON_DOWN && target < max) {
		target += 1;
		lcd.setCursor(0, 1);
		if (digit) {
			if (target < 0)
				lcd.print("-");
			lcd.print(abs(target) / 10);
			lcd.print(".");
			lcd.print(abs(target) % 10);
		}
		else
			lcd.print(target);
		lcd.print("      ");
#ifdef SERIAL_LCD
		Serial.println(target);
#endif
	}
	if (button & BUTTON_UP && target > min) {
		target -= 1;
		lcd.setCursor(0, 1);
		if (digit) {
			if (target < 0)
				lcd.print("-");
			lcd.print(abs(target) / 10);
			lcd.print(".");
			lcd.print(abs(target) % 10);
		}
		else
			lcd.print(target);
		lcd.print("      ");
#ifdef SERIAL_LCD
		Serial.println(target);
#endif
	}
	if (button & BUTTON_SELECT) {
		edit_num_target = NULL;
		return 0;
	}
	return WAIT;
} // }}}
MenuItem(back) {	// {{{
	return BACK;
} // }}}

// Startup Menu. {{{
MenuItem(toggle_winder) {	// {{{
	force_winder = !force_winder;
	return 0;
} // }}}
MenuItem(toggle_heater) {	// {{{
	force_heater = !force_heater;
	return 0;
} // }}}
MenuItem(toggle_auger) {	// {{{
	auger_on = !auger_on;
	return 0;
} // }}}
MenuItem(startup_back) {	// {{{
	if (force_winder)
		force_winder = -1;
	return BACK;
} // }}}
static Menu <4> startup_menu(	"Startup" , (char const *[]){"Toggle Heater", "Toggle Winder", "Toggle Auger", "Back"}, (action *[]){&toggle_heater, &toggle_winder, &toggle_auger, &startup_back});
//				"........#170.5°C"
// }}}

// Heater Menu. {{{
MenuItem(temp_set) {	// {{{
	static int T;
	if (!edit_num_target) {
		T = T_sp;
	}
	uint8_t ret = edit_num(T, "Set temp target", 0, 500, false);
	T_sp = T;
	return ret;
} // }}}
MenuItem(pid_p) {	// {{{
	int ret = edit_num(T_KP, "Proportional(%/K)", 0, 200, false);
	T_PID.SetTunings(T_KP, T_KI / 1000., T_KD);
	return ret;
} // }}}
MenuItem(pid_i) {	// {{{
	int ret = edit_num(T_KI, "I(.01%/Ks)", 0, 200, true);
	T_PID.SetTunings(T_KP, T_KI / 1000., T_KD);
	return ret;
} // }}}
MenuItem(pid_d) {	// {{{
	int ret = edit_num(T_KD, "Derivative(s%/K)", 0, 200, false);
	T_PID.SetTunings(T_KP, T_KI / 1000., T_KD);
	return ret;
} // }}}
static Menu <4> pid_menu(	"PID"     , (char const *[]){"Proportional", "Integration", "Derivative", "Back"}, (action *[]){&pid_p, &pid_i, &pid_d, &back});
static Menu <3> temp_menu(	"Heater"  , (char const *[]){"Set Target", "Set PID", "Back"}, (action *[]){&temp_set, &pid_menu, &back});
//				"........#170.5°C"
// }}}

// Guide Menu. {{{
MenuItem(guide_set_min) {	// {{{
	guide_current = guide_min;
	int setting = guide_current;
	uint8_t ret = edit_num(setting, "Set Min Guide", -90, 90, false);
	guide_min = setting;
	guide_current = setting;
	set_guide(guide_current);
	return ret;
} // }}}
MenuItem(guide_set_max) {	// {{{
	guide_current = guide_max;
	int setting = guide_current;
	uint8_t ret = edit_num(setting, "Set Max Guide", -90, 90, false);
	guide_max = setting;
	guide_current = setting;
	set_guide(guide_current);
	return ret;
} // }}}
MenuItem(guide_set_steps) {	// {{{
	return edit_num(guide_steps, "Set Guide Steps", 0, 255, false);
} // }}}
static Menu <4> guide_menu(	"Guide"     , (char const *[]){"Set Right", "Set Left", "Set # of Steps", "Back"}, (action *[]){&guide_set_min, &guide_set_max, &guide_set_steps, &back});
// }}}

// QC Menu. {{{
MenuItem(qc_report) {	// {{{
	if (checkButton(false) & BUTTON_SELECT)
		return 0;
	lcd.setCursor(0, 0);
	lcd.print("Diameter:");
	lcd.print(diameter);
	lcd.print("mm     ");
	lcd.setCursor(0, 1);
	lcd.print("Length:");
	lcd.print(total_length / 10.);
	lcd.print("m     ");
	return WAIT;
} // }}}
MenuItem(qc_dia_slope) {	// {{{
	return edit_num(dia_slope, "Dia Slope", -100, 100, true);
} // }}}
MenuItem(qc_dia_intercept) {	// {{{
	return edit_num(dia_intercept, "Dia Intercept", -100, 100, true);
} // }}}
MenuItem(qc_len_step) {	// {{{
	return edit_num(len_step, "Length Step", 0, 1000, true);
} // }}}
static Menu <5> qc_menu(	"QC"      , (char const *[]){"Report", "Dia Slope", "Dia Intercept", "Length Step", "Back"}, (action *[]){&qc_report, &qc_dia_slope, &qc_dia_intercept, &qc_len_step, &back});
//				"........#170.5°C"
// }}}

template <typename T> void save_item(int &addr, T const &item) {	// {{{
	for (unsigned int i = 0; i < sizeof(T); ++i)
		EEPROM.write(addr++, (reinterpret_cast <char const *>(&item))[i]);
} // }}}
template <typename T> void load_item(int &addr, T &item) {	// {{{
	union {
		char data[sizeof(T)];
		T result;
	} convert;
	for (unsigned int i = 0; i < sizeof(T); ++i)
		convert.data[i] = EEPROM.read(addr++);
	item = convert.result;
} // }}}
MenuItem(save) {	// {{{
	int addr = 0;
	EEPROM.write(addr++, EEPROM_MAGIC);	// Magic marker to detect valid data.
	save_item(addr, auger_on);
	save_item(addr, T_KP);
	save_item(addr, T_KI);
	save_item(addr, T_KD);
	save_item(addr, beta);
	save_item(addr, T_sp);
	save_item(addr, guide_min);
	save_item(addr, guide_max);
	save_item(addr, guide_steps);
	save_item(addr, dia_slope);
	save_item(addr, dia_intercept);
	save_item(addr, len_step);
	return BACK;
} // }}}
static void load() { // {{{
	int addr = 0;
	if (EEPROM.read(addr++) != EEPROM_MAGIC)
		return;
	load_item(addr, auger_on);
	load_item(addr, T_KP);
	load_item(addr, T_KI);
	load_item(addr, T_KD);
	load_item(addr, beta);
	load_item(addr, T_sp);
	load_item(addr, guide_min);
	load_item(addr, guide_max);
	load_item(addr, guide_steps);
	load_item(addr, dia_slope);
	load_item(addr, dia_intercept);
	load_item(addr, len_step);
} // }}}
static Menu <5> main_menu(	""        , (char const *[]){"Startup", "Heater", "Guide", "QC", "Save"}, (action *[]){&startup_menu, &temp_menu, &guide_menu, &qc_menu, &save});
//				"........#170.5°C"
// }}}
// Heater. {{{
static float current_temp;

static float read_temp() {
	raw_temp = analogRead(PIN_TEMP);
#ifdef THERMOCOUPLE
	raw_temp * TEMP_A + TEMP_B;
#else
	return -beta / log(K * ((1 << ADCBITS) / R0 / raw_temp - 1 / R0 - 1 / R1)) - 273.15;
#endif
}
// }}}
// Auger motor. {{{
// }}}
// QC. {{{
static void update_len() { // {{{
	len_hall_state = digitalRead(PIN_LEN);
	if (len_hall_state == LOW && len_hall_state != last_len_hall_state)
		total_length += len_step;
	last_len_hall_state = len_hall_state;
} // }}}
// }}}
// Serial. {{{
static void handle_serial() { // {{{
	static char buffer[12];
	static uint8_t pos = 0;
	if (!Serial.available())
		return;
	buffer[pos] = Serial.read();
	if (buffer[pos] == '\r')
		buffer[pos] = '\n';
	Serial.write(buffer[pos]);
	if (buffer[pos] == '\n') {
		// Handle command.
		buffer[pos] = 0;
		pos = 0;
		if (buffer[0] == 0) {
			send_status = !send_status;
			if (send_status)
				Serial.println("Length\tDiam\tTemp\tADC\tPID\tITerm");
			else
				Serial.println("Status output disabled.");
			return;
		}
		if (buffer[0] == '?' && buffer[1] == 0) {
			Serial.print("*=");
			Serial.print(auger_on);
			Serial.println("\tAuger on/off");

			Serial.print("P=");
			Serial.print(T_KP);
			Serial.println("\tPID Proportional");

			Serial.print("I=");
			Serial.print(T_KI / 1000.);
			Serial.println("\tPID Integrating");

			Serial.print("D=");
			Serial.print(T_KD);
			Serial.println("\tPID Derivative");

			Serial.print("B=");
			Serial.print(beta);
			Serial.println("\tβ for thermistor");

			Serial.print("T=");
			Serial.print(T_sp);
			Serial.println("\tTemperature target");

			Serial.print("+=");
			Serial.print(guide_max);
			Serial.println("\tLeft Guide Limit");

			Serial.print("-=");
			Serial.print(guide_min);
			Serial.println("\tRight Guide Limit");

			Serial.print("==");
			Serial.print(guide_current);
			Serial.println("\tCurrent Guide Position");

			Serial.print("_=");
			Serial.print(guide_direction);
			Serial.println("\tCurrent Guide Direction");

			Serial.print("#=");
			Serial.print(guide_steps);
			Serial.println("\tSteps for Full Range of Guide");

			Serial.print("a=");
			Serial.print(dia_slope / 10.);
			Serial.println("\tSlope of QC Diameter Calibration Fit");

			Serial.print("b=");
			Serial.print(dia_intercept / 10.);
			Serial.println("\tOffset of QC Diameter Calibration Fit");

			Serial.print("l=");
			Serial.print(len_step / 10.);
			Serial.println("\tLength per QC Revolution");

			Serial.print("L=");
			Serial.print(total_length / 10.);
			Serial.println("\tTotal Length Since Startup");

			return;
		}
		char var;
		int v1, v2 = 0;
		uint8_t n = sscanf(buffer, "%c=%de%d", &var, &v1, &v2);
		if (n < 2) {
			Serial.println("Ignoring invalid command.");
			return;
		}
		float value;
		value = v1;
		while (v2 > 0) {
			v2 -= 1;
			value *= 10;
		}
		while (v2 < 0) {
			v2 += 1;
			value /= 10;
			dbg("New value:", value);
		}
		switch(var) {
		case '*':
			auger_on = value;
			break;
		case 'P':
			T_KP = value;
			T_PID.SetTunings(T_KP, T_KI / 1000., T_KD);
			break;
		case 'I':
			T_KI = value * 1000;
			T_PID.SetTunings(T_KP, T_KI / 1000., T_KD);
			break;
		case 'D':
			T_KD = value;
			T_PID.SetTunings(T_KP, T_KI / 1000., T_KD);
			break;
		case 'B':
			beta = value;
			break;
		case 'T':
			T_sp = value;
			break;
		case '+':
			guide_max = value;
			break;
		case '-':
			guide_min = value;
			break;
		case '=':
			guide_current = value;
			set_guide(guide_current);
			break;
		case '_':
			guide_direction = value;
			break;
		case '#':
			guide_steps = value;
			break;
		case 'a':
			dia_slope = value * 10;
			break;
		case 'b':
			dia_intercept = value * 10;
			break;
		case 'l':
			len_step = value * 10;
			break;
		case 'L':
			total_length = value * 10;
			break;
		default:
			Serial.println("Invalid variable");
			break;
		}
	}
	else if (pos >= sizeof(buffer)) {
		Serial.println("\nBuffer overflow; command ignored.");
		pos = 0;
	}
	else
		pos += 1;
} // }}}
// }}}

/*********** Main functions. **************/
void setup() { // {{{
	// Before doing anything else: make sure MOSFET inputs are not floating.
	pinMode(PIN_WINDER, OUTPUT);
	digitalWrite(PIN_WINDER, LOW);
	pinMode(PIN_AUGER, OUTPUT);
	digitalWrite(PIN_AUGER, INVERT_AUGER(false));
	pinMode(PIN_HEATER, OUTPUT);
	digitalWrite(PIN_HEATER, INVERT_HEATER(false));

	Serial.begin(115200);
	uint8_t mcusr = MCUSR;
	MCUSR = 0;
	dbg("starting; mcusr=", mcusr);
	// Restore settings from EEPROM.
	load();
	// UI.
	init_lcd();
	pinMode(PIN_A, INPUT_PULLUP);
	pinMode(PIN_B, INPUT_PULLUP);
	pinMode(PIN_BUTTON, INPUT_PULLUP);
	// Pin change interrupts are set up below.
	// Heater.
	T_sp = INITIAL_TEMP_TARGET;
	T_PID.SetOutputLimits(T_OP_MIN, T_OP_MAX);
	T_PID.SetSampleTime(T_SAMPLE_TIME);
	T_PID.SetMode(1);
	heater_output = 0;
	last_temp_update = millis();
	// PIN_TEMP is analog.
	// Winder.
	pinMode(PIN_HALL, INPUT_PULLUP);
	pinMode(PIN_GUIDE, OUTPUT);
	// Set up timer1 for servo control.
	TCCR1A = 0x82;
	TCCR1B = 0x1a;
	ICR1H = GUIDE_TICKS_PER_PERIOD >> 8;
	ICR1L = GUIDE_TICKS_PER_PERIOD & 0xff;
	TCNT1H = 0;
	TCNT1L = 0;
	pinMode(SS, OUTPUT);
	digitalWrite(SS, LOW);
	pinMode(SCL, INPUT_PULLUP);
	pinMode(SDA, INPUT_PULLUP);
	// QC.
	// PIN_DIA is analog.
	pinMode(PIN_LEN, INPUT_PULLUP);
	PCMSK2 = PIN_A_MASK | PIN_B_MASK | PIN_BUTTON_MASK;
	PCIFR = 0xff;
	PCICR = 1 << PC_CONTROL_BIT;
	sensor_sync_phase = 1;
	// Reset controls.
	clicked = false;
	delta = 0;
	guide_current = (guide_min + guide_max) / 2;
	set_guide(guide_current);
	last_report = millis();
	dbg("setup done", "");
} // }}}

void loop() { // {{{
	current_temp = read_temp();
	bool run_auger = auger_on && current_temp >= MIN_TEMP && current_temp < MAX_TEMP;
	char buffer[15];
	uint8_t pos = 0;
	char indicator = ((force_heater && T_op > 0.) << 2) | ((run_auger) << 1) | ((auger_on) << 0);
	buffer[pos++] = indicator ? indicator : 2;	// Use otherwise unused character 2 to avoid triggering end of string.
	if (current_temp < 0)
		buffer[pos++] = '-';
	pos += snprintf(buffer + pos, sizeof(buffer) - pos - 1, "%d.", int(abs(current_temp)));
	buffer[pos++] = '0' + int(abs(current_temp) * 10) % 10;
	pos += snprintf(buffer + pos, sizeof(buffer) - pos - 1, "/%d", int(T_sp));
	buffer[pos++] = 0;
	// UI.
	unsigned long now = millis();
	main_menu.iteration(false, buffer);
	// Heater.
	T_ip = current_temp;
	if (!force_heater) {
		heater_duty = 0;
		digitalWrite(PIN_HEATER, INVERT_HEATER(false));
	}
	else if (T_PID.Compute()) {
		//dbg("pid out: ", T_op);
		heater_output = T_op;
		if (heater_output <= 0)
			heater_output = 0;
		else if (heater_output >= 100)
			heater_output = 100;
		heater_duty = heater_output;
	}
	if (now - last_temp_update >= 100) {
		// Manual pwm of heater.
		digitalWrite(PIN_HEATER, INVERT_HEATER(heater_duty > heater_phase));
		heater_phase += 1;
		heater_phase %= 100;
		while (now - last_temp_update >= 100)
			last_temp_update += 100;
	}
	// Winder.
	analogWrite(PIN_WINDER, handle_winder());
	handle_guide();
	// Auger motor.
	digitalWrite(PIN_AUGER, INVERT_AUGER(run_auger));
	// QC.
	update_len();
	// Report once per second.
	if (!sensor_sync_phase && now - last_report >= 1000) {
		diameter = (analogRead(PIN_DIA) - dia_intercept / 10.) / (dia_slope / 10.);
		if (send_status) {
			Serial.print(total_length / 10.);
			Serial.print("\t");
			Serial.print(diameter);
			Serial.print("\t");
			Serial.print(current_temp);
			Serial.print("\t");
			Serial.print(raw_temp);
			Serial.print("\t");
			Serial.print(T_op);
			Serial.print("\t");
			Serial.print(T_PID.GetITerm());
			Serial.print("\n");
		}
		while (now - last_report >= 1000)
			last_report += 1000;
	}
	handle_serial();
} // }}}
