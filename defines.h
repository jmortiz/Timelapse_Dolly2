//stepper pins
#define DIR_PIN 12
#define STEP_PIN 11
#define STEPPER_ENABLE_PIN 13

//limit switches
#define HOME_PIN 2
#define AWAY_PIN 3

//camera pins
#define FOCUS_PIN A2
#define SHUTTER_PIN A1

//program states
#define STATE_INIT 0
#define STATE_ASK_HOME 1
#define STATE_GO_HOME 2
#define STATE_ASK_INTERVAL 3
#define STATE_ASK_TOTAL_TIME 4
#define STATE_ASK_START 5
#define STATE_TIMELAPSE 6
#define STATE_PAUSE 7
#define STATE_ASK_CANCEL 8

//constraints
#define MIN_INTERVAL 2.0 //seconds
#define MAX_INTERVAL 60.0 //seconds
#define MIN_DELTAX 1.0 //milimeters
#define MAX_FOCUS_TIME 1.0 //miliseconds
#define TOTAL_LENGTH 1000.0 //milimeters
#define STEPS_PER_MM 161.3
