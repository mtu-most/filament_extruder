// Firmware for an Arduino to be used for reading debugging info from the sensor.
volatile uint8_t buffer[768];
volatile int pixel = 768;
volatile int pixels = -1;
volatile int pixel_state = 0;

ISR(SPI_STC_vect) {
	if (pixel_state == 0) {
		pixels = SPDR & 0xff;
		pixel_state += 2;
		pixel = 0;
		return;
	}
	if (pixel_state == 1) {
		pixels |= int(uint8_t(SPDR)) << 8;
		pixel_state += 1;
		pixel = 0;
		if (pixels > 768)
			pixels = -1;
		return;
	}
	if (pixel >= pixels)
		return;
	buffer[pixel] = SPDR;
	//Serial.print(buffer[pixel]);
	pixel += 1;
}

void sync_sensor() {
	//Serial.println("sync");
	pinMode(SCK, INPUT);
	pinMode(MISO, OUTPUT);
	digitalWrite(MISO, LOW);
	SPCR = 0x00;
	delay(500);
	pixel_state = 0;
	SPCR = 0xe0;
	pinMode(MISO, INPUT);
}

void setup() {
	Serial.begin(115200);
	sync_sensor();
}

void loop() {
	if (pixel_state == 2 && pixels > 0 && pixel >= pixels) {
		//Serial.println("data");
		Serial.write('\x55');
		Serial.write(uint8_t(pixels & 0xff));
		Serial.write(uint8_t(pixels >> 8));
		for (int i = 0; i < pixels; ++i)
			Serial.write(buffer[i]);
		sync_sensor();
	}
}
