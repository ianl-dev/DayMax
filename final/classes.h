#include <WiFi.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <string.h>
#include <math.h>

const uint16_t LARGE_BUFFER_SIZE = 300;
const uint16_t SMALL_BUFFER_SIZE = 100;
const uint16_t DATA_SIZE = 100;

/*
 * Simple struct to represent time
 */
typedef struct _time{
  int hour;
  int minute;
  int second;
  int cmp(struct _time other); //number of milliseconds between other and this, positive if other comes after this
  struct _time forward(int timeForward); //time some milliseconds forward
  void toString(char* buffer); // to string method (with space, i.e. " 5:40" vs "5:40")
  void toShortString(char* buffer); // to string method (without space)
} Time;


/*
 * Class for drawing/writing to LCD
 * Has two writing/drawing areas
 * Supports scrolling through lines in the body area
 */
class LCD{
public:
  char body_str[LARGE_BUFFER_SIZE];
  char header_str[LARGE_BUFFER_SIZE];
  Time displayedTime;
  char ffs[LARGE_BUFFER_SIZE];
  char draw_buffer[LARGE_BUFFER_SIZE];
  char temp_buffer[LARGE_BUFFER_SIZE];
  float curScrollExact = 0;
  int frozenLines = 0, curScrollAmount = 0, scrollAmount = 0, linePointer = 0, curPointer = 0, lastUpdate = 0, bodyLines = 0, pointerUpdate; // scrolling stuff
  const int startingBodyHeight = 26, endingBodyHeight = 152, lineHeight = 14;
  bool showPointer = false;
  TFT_eSPI tft = TFT_eSPI();
  LCD(); // initialization
  void write_body(char* stuff);
  void write_header(char* stuff);
  void write_time(Time stuff);
  void clear_body();
  void display_body();
  void display_header();
  void display_time();
  void setPointer(bool pointerSetting);
  void scrollLines(int dir); // dir = +1 to scroll down, -1 to scroll up
  void resetLines(int newFrozen); // The first newFrozen lines are frozen when scrolling
  void display(); // display all
};

/*
 * Class for buzzer/LED alarm
 * Only handles blaring, doesn't understand time
 * Pins hardcoded
 */
class Alarm{
//  int redPin = 12, greenPin = 27, bluePin = 26;
//  int redVal = 0, greenVal = 0, blueVal = 0;
  //duration is how long the alarm should be blaring for at most, but it's better to handle that stuff in the main fsm
  uint32_t startTime = 0, period = 2000, duration = 600000;
//  bool blare = false;
public:
  Alarm(); // initialization
  void start(); // turn on buzzer
  void stop();
//  void flip(); // flip on/off
//  void apply(); // blare
//  bool isBlaring(); // check if on/off
};

/*
 * Button class from weekly exercise
 */
class Button{
  public:
  uint32_t t_of_state_2;
  uint32_t t_of_button_change;    
  uint32_t debounce_time;
  uint32_t long_press_time;
  uint8_t pin;
  uint8_t flag;
  bool button_pressed;
  uint8_t state;
  Button(int p);
  void read();
  int update(); // 1 for short press, 2 for long press
};

/*
 * Struct representing important info about user
 */
typedef struct _user{  
  char name[LARGE_BUFFER_SIZE];
  char email[LARGE_BUFFER_SIZE];
  int age;
  char gender;

  int bloodPressure[DATA_SIZE]; // measured blood pressure over number of days, guess we'll be changing it to heart rate
  
  char providerPrefix[LARGE_BUFFER_SIZE];
  char providerName[LARGE_BUFFER_SIZE];
  char providerEmail[LARGE_BUFFER_SIZE];
} User;

typedef struct {
  char med_name[LARGE_BUFFER_SIZE];

  int dosage;
  Time times[DATA_SIZE];
  int handling[DATA_SIZE];
  
  int timeNum = 0;
  int todayNum = 0;

  bool prereqMeal = false;
  char prereqMealDetail;//'A' = After, 'B' = Before, 'D' = During

  bool prereqTemp = false;
  char prereqTempDetail;

  void reset(); // reset class attributes
  void assignInfo(char* infoBuffer); // assign class attributes from GET response buffer
  
  int getPrereqPrompt(int cur, char* prompt);
  
  void addTime(Time newTime);
  
  bool isTaken();
} Medication;

/*
 * Struct that keeps track of current time
 * Only retrieves world time (with API call) every hour or so, usually uses millis() + lastTime
 */
typedef struct _clock{
  uint32_t counter = 0;
  Time lastTime = {0, 0, 0};
  void updateTime();
  Time getTime();
} Clock;

/*
 * Struct that stores taking-medicine-events
 */
 typedef struct _event{
  int medicineIndex; // i.e. index in medicine array
  int timeIndex; // i.e. index in medicine.times array
  Time eventTime;
 } Event;
