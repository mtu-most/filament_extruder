// Firmware for an Arduino to be used for reading debugging info from the sensor.
volatile int pixel_state = 0;
volatile int pixels = -1;
volatile int current = 80;

ISR(SPI_STC_vect) {
	int d = SPDR & 0xff;
	if (pixel_state == 3)
		return;
	if (pixel_state == 0) {
		pixels = d;
		pixel_state = 1;
		return;
	}
	if (pixel_state == 1) {
		current = d;
		pixel_state = 2;
		return;
	}
	if (d != 0x53)
		pixel_state = 3;
	else
		pixel_state = 1;
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
	while (pixel_state == 0) {}
}

void setup() {
	Serial.begin(115200);
	sync_sensor();
}

void loop() {
	if (pixel_state == 3)
		sync_sensor();
	Serial.println(current);
}
