#include <elapsedMillis.h>

elapsedMillis time;
int analogValue = 10;
bool led = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(0, OUTPUT);

}

void loop() {
  analogValue = analogRead(2) / 16 + 1;
  if (time > analogValue) {
    time = 0;
    led = !led;
    digitalWrite(0, led ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level)
  }
}
