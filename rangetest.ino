// ---------------------------------------------------------------------------
// This example code was used to successfully communicate with 15 ultrasonic sensors. You can adjust
// the number of sensors in your project by changing SONAR_NUM and the number of NewPing objects in the
// "sonar" array. You also need to change the pins for each sensor for the NewPing objects. Each sensor
// is pinged at 33ms intervals. So, one cycle of all sensors takes 495ms (33 * 15 = 495ms). The results
// are sent to the "oneSensorCycle" function which currently just displays the distance data. Your project
// would normally process the sensor results in this function (for example, decide if a robot needs to
// turn and call the turn function). Keep in mind this example is event-driven. Your complete sketch needs
// to be written so there's no "delay" commands and the loop() cycles at faster than a 33ms rate. If other
// processes take longer than 33ms, you'll need to increase PING_INTERVAL so it doesn't get behind.
// ---------------------------------------------------------------------------
#include <NewPing.h>
#include <math.h>

#define SONAR_NUM     2 // Number or sensors.
#define MAX_DISTANCE  60 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 60 // slowed down to be able to read on console :) 33 Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

int greenLed = 11;
int redLed = 10;
unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned int cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

struct coord_t
{
  unsigned int x;
  unsigned int y;
};

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  //NewPing(13, 12, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(5, 6, MAX_DISTANCE),
  NewPing(8, 7, MAX_DISTANCE)/*,
  NewPing(21, 22, MAX_DISTANCE),
  NewPing(23, 24, MAX_DISTANCE),
  NewPing(25, 26, MAX_DISTANCE),
  NewPing(27, 28, MAX_DISTANCE),
  NewPing(29, 30, MAX_DISTANCE),
  NewPing(31, 32, MAX_DISTANCE),
  NewPing(34, 33, MAX_DISTANCE),
  NewPing(35, 36, MAX_DISTANCE),
  NewPing(37, 38, MAX_DISTANCE),
  NewPing(39, 40, MAX_DISTANCE),
  NewPing(50, 51, MAX_DISTANCE),
  NewPing(52, 53, MAX_DISTANCE)*/
};

static long sqr (unsigned int x) { return x * (long)x; }

struct coord_t translate (unsigned int x, unsigned int z1, unsigned int z2)
{
  unsigned long x_out = abs ((sqr(x) - sqr(z1) + sqr(z2)) / (2 * x));
  unsigned long y_out = sqrt (abs (sqr(z2) - sqr(x_out)));
  coord_t res = { x_out, y_out };
  return res;
}

int was_good_hit (struct coord_t pt)
{
  return (pt.y >= 130 && pt.y <= 200) &&
         (pt.x >= 130 && pt.x <= 200);
}

int was_a_hit (unsigned int a, unsigned int b) {
  return !((a == 0 || a > 450)  && (b == 0 || b > 450));
}


void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;

  pinMode(greenLed, OUTPUT); 
  pinMode(redLed, OUTPUT);  
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW); 
}

void loop() {
  
  for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
      if (i == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
      sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                          // Sensor being accessed.
      cm[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
      sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
    }
  }
  // The rest of your code would go here.
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / (US_ROUNDTRIP_CM / 10);
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
  #if 0
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    Serial.print(i);
    Serial.print("=");
    Serial.print(cm[i]);
    Serial.print("mm ");
  }
  #endif
  unsigned int x = 310;
  struct coord_t pt = translate (x, cm[0], cm[1]);
#if 1
    Serial.print("x=");
    Serial.print(pt.x);
    Serial.print("mm, y=");
    Serial.print(pt.y);
    Serial.print("mm ");
#endif

  //Serial.println();
  static int cycles = 0, hits = 0, good = 0;

  int hit = was_a_hit (cm[0], cm[1]) ? 1 : 0;
  hits += hit;
  if (hit)
    good += was_good_hit (pt) ? 1 : 0;

#define CYCLE_SPAN 3
#define CYCLE_MIN_HITS 2
#define CYCLE_MIN_GOOD 2

  if (++cycles < CYCLE_SPAN)
    ;//return;

  if(was_a_hit(cm[0], cm[1])) {
  //if (hits >= CYCLE_MIN_HITS) {
    if(was_good_hit(pt)) {
    //if (good >= CYCLE_MIN_GOOD) {
      digitalWrite(greenLed, LOW);
      digitalWrite(redLed, HIGH);  
    }
    else {
      digitalWrite(greenLed, HIGH);
      digitalWrite(redLed, LOW);  
    }
  }
  else {
      digitalWrite(greenLed, HIGH);
      digitalWrite(redLed, HIGH);  
  }

  cycles = hits = good = 0;
}
