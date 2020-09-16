#include "classes.h"
#include <mpu6050_esp32.h>

/* state definition */
#define IDLE 0
#define RING 1
#define PROMPT1 2
#define PROMPT2 3
#define SUCCESS 4
#define SUCCESS_WAIT 5

#define MENU_OFF 0
#define MENU_MAIN 1
#define MENU_UPCOMING 2
#define MENU_MISSED 3
#define MENU_MET 4
#define MENU_PENDING 5
#define MENU_REPORT 6

// input state only accessible through measure state
#define MENU_MEASURE 7
#define MENU_INPUT 8

/*  --------------- wifi ------------------ */
//char network[] = "!";
//char pass[] = "amiibiia";

//char network[] = "BELL929";
//char pass[] = "DF3E511A91A5";

char network[] = "NETGEAR38";
char pass[] = "icyvase763";
/*  --------------------------------------- */

// update timer details
const int32_t ONE_MINUTE = 60000;
const uint8_t LOOP_PERIOD = 10;
const uint32_t UPDATE_PERIOD = 60000;

uint32_t waiting_timer = 0;
uint32_t primary_timer = 0;
uint32_t secondary_timer = 0;
uint32_t tertiary_timer = 0;
uint32_t another_timer = 0;
uint32_t whatever_timer = 0;

// request buffer details
const int RESPONSE_TIMEOUT = 6000;
const int POSTING_PERIOD = 6000;
const uint16_t IN_BUFFER_SIZE = 500;
const uint16_t OUT_BUFFER_SIZE = 500;
char request_buffer[IN_BUFFER_SIZE];
char response_buffer[OUT_BUFFER_SIZE];

char prompt_buffer[OUT_BUFFER_SIZE];
char list_buffer[OUT_BUFFER_SIZE];
char temp_buffer[OUT_BUFFER_SIZE];

// Late_med - updating late medication
char late_med[50];

/* ----- IMU - updating health conditions ----- */
MPU6050 imu; //imu object for scrolling
bool start_tilt = false; // IMU
bool cond_status = false;

char heart[10];
char respiration[10];
char systolic[10];
char diastolic[10];
char blood_sugar[10];

bool is_heart = false;
bool is_respiration = false;
bool is_systolic = false;
bool is_diastolic = false;
bool is_blood_sugar = false;

float angle;
unsigned long scrolling_timer;
int num_index = 0;
char query[50] = {0};

/* -------------------------------------------- */

int state = IDLE, menu_state = MENU_OFF;

Button button1 = Button(16), button2 = Button(5);

/* ----- Create alarm, clock, screen, and user objects ----- */
Alarm buzzer = Alarm(); // name changed from "led" to "buzzer"
Clock ticker;
LCD screen = LCD();
User user;
char username[] = "Bob Dylan";

// initialize medication list
Medication medicine[25];
int medicineNum = 0;
Medication currentMedicine;
int prereqNum = 0;

int lastSnoozed = -60 * ONE_MINUTE;

bool started = false; // init
/* --------------------------------------------------------- */

void setup() {
  // Buttons settings
  pinMode(16, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  Serial.begin(115200);

  // IMU
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }
  
  // WiFi
  WiFi.begin(network, pass);
  uint8_t count = 0;
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  Serial.println(pass);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) {
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else {
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart();
  }
  
  // Get user and medication data
  initUser();
  initMedication();

  ticker.counter = millis();
  ticker.updateTime();

  currentMedicine = getNext(ticker.getTime());
}

const int INIT_GET_DELAY = 30000;

void initUser(){
  sprintf(user.name, username);

  while (true) {
    // loop if there's no user data in the database, then try checking every
    // 30 seconds. Exit loop when user data exists.
    sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/get_basic_info.py?name=%s HTTP/1.1\r\n", user.name);
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(request_buffer, "\r\n");
    strcat(request_buffer, "\r\n");
    Serial.println(request_buffer);
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

    char no_user[OUT_BUFFER_SIZE] = "No user found\n";
    if (strcmp (response_buffer,no_user) != 0) { // if user is found
      char *pch;
      pch = strtok (response_buffer, ",");
      int count = 0;
      // count = 0 -> name, 1 -> email, 2 -> provider's last name, 3 -> provider's email
      while (pch != NULL) {
        if (count == 1) sprintf(user.email, pch);
        else if (count == 2) sprintf(user.providerName, pch);
        else if (count == 3) sprintf(user.providerEmail, pch);
        count++;
        pch = strtok(NULL, ",");
      }
      Serial.printf("User Init success: %s %s %s %s\n", user.name, user.email, user.providerName, user.providerEmail);
      break;
    }
    delay(INIT_GET_DELAY);
  }
}

void initMedication(){
  Serial.println("Initializing medication");
  while(medicineNum == 0){
    get_med_info();
    delay(INIT_GET_DELAY);
  }
}

void loop() {
//  uncomment for LED
//  buzzer.apply(); 
  Time now = ticker.getTime();
  if(started){
    screen.write_time(now);
    screen.display();
    fsm();
  }

  if(start_tilt){
    float x, y;
    get_angle(&x, &y); //get angle values
    angle = -y;
  }
  
  while (millis() - primary_timer < LOOP_PERIOD);
  primary_timer = millis();
  if(millis() - secondary_timer > UPDATE_PERIOD && now.second < 5){
      started = true;
      get_med_info();
      if(state == IDLE){
        currentMedicine = getNext(now.forward(-30 * ONE_MINUTE));
        whatever_timer = millis();
      }
      updateHeader();
      secondary_timer = millis();
  }
}

void updateHeader(){
  Time now = ticker.getTime();
  
  int missingNum = getMissedNum(now);
  int pendingNum = getPendingNum(now);
  
  sprintf(screen.header_str, "%d ?   %d !", pendingNum, missingNum);
}

void fsm(){
  int state1 = button1.update();
  int state2 = button2.update();
  Time now = ticker.getTime();
  
  if(state == IDLE){
    buzzer.stop();
//  uncomment for LED
//  buzzer.stop();
    if(now.cmp(currentMedicine.times[0]) < 0 && currentMedicine.dosage != -1) {
      if(millis() - lastSnoozed < 10 * ONE_MINUTE && millis() - lastSnoozed > 0){
        sprintf(screen.body_str, "The alarm for\n%s is\nsnoozed.", currentMedicine.med_name);
      }
      else{
        state = RING;
        buzzer.start();
        buzzer.start();
        prereqNum = 0;
        waiting_timer = millis();
      }
    }
  }

  // entering the Menu page
  if(state1 == 2){
    if(menu_state != MENU_MAIN){
      menu_state = MENU_MAIN;
      screen.clear_body();
      screen.resetLines(1);
      screen.setPointer(true);
      another_timer = 0;
      return;
    }
    menu_state = MENU_OFF;
    screen.clear_body();
    screen.resetLines(0);
    screen.setPointer(false);
    return;
  }

  if(menu_state == MENU_MAIN){
    sprintf(screen.body_str, "Pick an option.\n   See missed\n   See met\n   See upcoming\n   See pending\n   Report\n   Measure");
    if(state2 == 2){
      if(screen.linePointer == 2){
        sprintf(screen.body_str, "Loading");
        menu_state = MENU_MISSED;
      }
      if(screen.linePointer == 3){
        sprintf(screen.body_str, "Loading");
        menu_state = MENU_MET;
      }
      if(screen.linePointer == 4){
        sprintf(screen.body_str, "Loading");
        menu_state = MENU_UPCOMING;
      }
      if(screen.linePointer == 5){
        sprintf(screen.body_str, "Loading");
        menu_state = MENU_PENDING;
      }
      if(screen.linePointer == 6){
        sprintf(screen.body_str, "Loading");
        menu_state = MENU_REPORT;
        // avoid redundant medicine listing
        memset(late_med, 0, sizeof(late_med));
        get_med_info(); // temp array always holds the latest med info
        get_late_med(now);
      }
      if(screen.linePointer == 7){
        sprintf(screen.body_str, "Loading");
        num_index = 0; // sanity check: make sure index is 0 every time user arrives to measure page
        sprintf(list_buffer, "non-empty");
        menu_state = MENU_MEASURE;
      }
      screen.clear_body();
      screen.resetLines(1);
      screen.setPointer(true);
      return;
    }
    if(state1 == 1){
      screen.scrollLines(1);
    }
    else if(state2 == 1){
      screen.scrollLines(-1);
    }
    return;
  }
 
  if(menu_state == MENU_MISSED){
    if(millis() - another_timer > 20000){
      getMissed(now);
      another_timer = millis();
    }
    if(strlen(list_buffer) == 0)
      sprintf(list_buffer, "\n   None");
    sprintf(screen.body_str, "Missed alarms:%s", list_buffer);
    if(state1 == 1){
      screen.scrollLines(1);
    }
    else if(state2 == 1){
      screen.scrollLines(-1);
    }
    return;
  }

  if(menu_state == MENU_MET){
    if(millis() - another_timer > 20000){
      getMet(now);
      another_timer = millis();
    }
    if(strlen(list_buffer) == 0)
      sprintf(list_buffer, "\n   None");
    sprintf(screen.body_str, "Met alarms:%s", list_buffer);
    if(state1 == 1){
      screen.scrollLines(1);
    }
    else if(state2 == 1){
      screen.scrollLines(-1);
    }
    return;
  }

  if(menu_state == MENU_UPCOMING){
    if(millis() - another_timer > 20000){
      getUpcoming(now);
      another_timer = millis();
    }
    if(strlen(list_buffer) == 0)
      sprintf(list_buffer, "\n   None");
    sprintf(screen.body_str, "Upcoming alarms:%s", list_buffer);
    if(state1 == 1){
      screen.scrollLines(1);
    }
    else if(state2 == 1){
      screen.scrollLines(-1);
    }
    return;
  }

  if(menu_state == MENU_PENDING){
    if(millis() - another_timer > 2000){
      getPending(now);
      another_timer = millis();
    }
    if(strlen(list_buffer) == 0)
      sprintf(list_buffer, "\n   None");
    sprintf(screen.body_str, "Pending alarms:%s", list_buffer);
    if(state1 == 1){
      screen.scrollLines(1);
    }
    else if(state2 == 1){
      screen.scrollLines(-1);
    }
    return;
  }

   if(menu_state == MENU_REPORT){ 
    if(strlen(list_buffer) == 0){
      sprintf(screen.body_str, "Status updated.");
      return;
    }
    sprintf(screen.body_str, "Which medication?%s", list_buffer);
    
    // CASE 1: long press to send/reset
    if (state2 == 2){
      // n_th line correspond to the n-2 th element in med_name e.g. 2nd line corresponds to the first medicine [0]
      int n = screen.linePointer;
      take_medicine(medicine[n-2].med_name, "late");
      screen.clear_body();
      sprintf(list_buffer, "");
    }
        
    // CASE 2: short press for navigation
    if(state1 == 1){
      screen.scrollLines(1);
    }
    else if(state2 == 1){
      screen.scrollLines(-1);
    }
    return;
  }

  if(menu_state == MENU_MEASURE){
    if(strlen(list_buffer) == 0 and cond_status){
      sprintf(screen.body_str, "Conditions updated.");
      memset(response_buffer, 0, sizeof(response_buffer));

      // reset all paramemters
      memset(heart, 0, sizeof(heart));
      memset(respiration, 0, sizeof(heart));
      memset(systolic, 0, sizeof(heart));
      memset(diastolic, 0, sizeof(heart));
      memset(blood_sugar, 0, sizeof(heart));
       
      return;
    }

    else if(strlen(list_buffer) == 0){
      sprintf(screen.body_str, "Failed. Please take \n all measurements.");
      return;
    }
    
    sprintf(screen.body_str, "Record measurement\n   Heart:%s\n   Respiration:%s\n   Systolic:%s\n   Diastolic:%s\n   Blood sugar:%s\n   -- Done --", heart, respiration, systolic, diastolic, blood_sugar);
    // CASE 1: Navigation
    if(state1 == 1){
      screen.scrollLines(1);
    }
    else if(state2 == 1){
      screen.scrollLines(-1);
    }

    // CASE 2: Long press B to update one condition
    if(state2 == 2){
      start_tilt = true;
      menu_state = MENU_INPUT;
      scrolling_timer = millis();
      screen.clear_body();
      memset(query, 0, sizeof(query));
            
      if(screen.linePointer == 2){  // Heart
        is_heart = true;
        memset(heart, 0, sizeof(heart));
      }
      if(screen.linePointer == 3){  // Respiration
        is_respiration = true;
        memset(respiration, 0, sizeof(respiration));
      }
      if(screen.linePointer == 4){  // Systolic
        is_systolic = true;
        memset(systolic, 0, sizeof(systolic));
      }
      if(screen.linePointer == 5){  // Diastolic
        is_diastolic = true;
        memset(diastolic, 0, sizeof(diastolic));
      }
      if(screen.linePointer == 6){  // Blood sugar
        is_blood_sugar = true;
        memset(blood_sugar, 0, sizeof(blood_sugar));
      }
      if(screen.linePointer == 7){  // DONE -> SEND ALL RECORDS

         // clear screen to displau message
         sprintf(list_buffer, "");
         screen.clear_body();
         menu_state = MENU_MEASURE;
         
         if (strlen(heart) > 0  // only allow send if all measurements are taken
            && strlen(respiration) > 0
            && strlen(systolic) > 0
            && strlen(diastolic) > 0
            && strlen(blood_sugar) > 0){
              update_condition(heart, respiration, systolic, diastolic, blood_sugar);
              cond_status = true;
              Serial.println(response_buffer);
            }
         else {
            cond_status = false;
         }
      }
    }
    return;
  }

  if(menu_state == MENU_INPUT){
    char nums[50] = "0123456789";
    
    const int scrolling_threshold = 400; // for better UI experience
    const float angle_threshold = 0.35;

    if(state1 == 2){
      menu_state = MENU_MEASURE;
      return;
    }

    // short press A modifies temp string
    if(state1 == 1){
        strncat(query, &nums[num_index], 1);       
        strcpy(screen.body_str, query);
        num_index = 0;
      }

    // long press B to store
    else if(state2 == 2){
      if(is_heart){
        sprintf(heart, query);
        is_heart = false;
      }
      else if(is_respiration){
        sprintf(respiration, query);
        is_respiration = false;
      }
      else if(is_systolic){
        sprintf(systolic, query);
        is_systolic = false;
      }
      else if(is_diastolic){
        sprintf(diastolic, query);
        is_diastolic = false;
      }
      else if(is_blood_sugar){
        sprintf(blood_sugar, query);
        is_blood_sugar = false;
      }
      memset(query, 0, sizeof(query));
      menu_state = MENU_MEASURE;
    }

    // keep looping
    else {
       if (millis()-scrolling_timer >= scrolling_threshold){  // select
         // LEFT
         if (angle < 0 and abs(angle) >= angle_threshold){
          scrolling_timer = millis();
          num_index--;
          // reset loop
          if(num_index < 1){
             num_index=9;
            }
          }
         // RIGHT 
         else if (angle > 0 and abs(angle) >= angle_threshold){
           scrolling_timer = millis();
           num_index++;
           // reset loop
           if(num_index > strlen(nums)-1){
             num_index=0;
           }
          }
       }
       // show current string
       strcpy(screen.body_str, query);
       strncat(screen.body_str, &nums[num_index], 1);
    }
    return;
  }
  
  if(menu_state != MENU_OFF){
    return;
  }
  
  if(state == IDLE){
    buzzer.stop();
//  uncomment for LED
//  buzzer.stop();
    if(currentMedicine.dosage == -1)
      sprintf(screen.body_str, "Hi %s!\nYou have no upcoming\nevents.", username);
    else{
      currentMedicine.times[0].toShortString(temp_buffer);
      sprintf(screen.body_str, "Hi %s!\nYour next medicine: \n%s at %s.", username, currentMedicine.med_name, temp_buffer);
    }
    //sprintf(screen.body_str, "Testing\n   1a\n   2b\n   3c\n   4d\n   5e\n   6f\n   7g\n   8h\n   9i\n   10j\n   11k\n   12l\n   13m\n   14n\n   15o\n   16p\n   17q\n   18r\n   19s\n   20t");
    if(now.cmp(currentMedicine.times[0]) > 0){
      return;
    }
    if(currentMedicine.dosage == -1) {
      return;
    }
    //currentMedicine.handling[0] = RINGING_MED;
    // Send email notification. Comment if you want to remove email notification feature
    //daily_notif_user(currentMedicine.med_name, currentMedicine.dosage);
    //                                          _    _    _
    //                            ^            | |  | |  | |
    //                          /   \          | |  | |  | |
    //                           | |           |_|  |_|  |_|
    //                           |_|           (_)  (_)  (_)
    //
    if(millis() - lastSnoozed < 10 * ONE_MINUTE && millis() - lastSnoozed > 0){
        sprintf(screen.body_str, "The alarm for\n%s is\nsnoozed.", currentMedicine.med_name);
        return;
    }
    state = RING;
    screen.clear_body();
    buzzer.start();
    prereqNum = 0;
    waiting_timer = millis();
  }
  
  else if(state == RING){
    bool snoozable = false;
    if((millis() - lastSnoozed) > (30 * ONE_MINUTE)) snoozable = true;
    sprintf(screen.body_str, "It's time for your\n%s.\nPress A to stop.", currentMedicine.med_name);
    if(snoozable){
      strcat(screen.body_str, "\nPress B to snooze");
    }
    if(millis() - waiting_timer > 300000){
      state = IDLE;
      return;
    }
    if(state1){
      waiting_timer = millis();
      state = PROMPT1;
      buzzer.stop();
      prereqNum = currentMedicine.getPrereqPrompt(prereqNum, prompt_buffer);
      screen.clear_body();
      if(prereqNum == -1){
        state = PROMPT2;
        screen.clear_body();
      }
      else{
        sprintf(screen.body_str, "%s", prompt_buffer);
      }
      return;
    }
    if(state2 && snoozable){
      //currentMedicine.handling[0] = SLEEPING_MED;
      lastSnoozed = millis();
      state = IDLE;
      buzzer.stop();
      screen.clear_body();
    }
  }
  
  else if(state == PROMPT1){
    if(millis() - waiting_timer > 60000){
      state = RING;
      buzzer.start();
      return;
    }
    if(state1){
      waiting_timer = millis();
      prereqNum = currentMedicine.getPrereqPrompt(prereqNum, prompt_buffer);
      if(prereqNum == -1){
        state = PROMPT2;
        screen.clear_body();
      }
      else{
        sprintf(screen.body_str, "%s", prompt_buffer);
      }
    }
    else if(state2){
      waiting_timer = millis();
      state = RING;
      screen.clear_body();
      buzzer.start();
      prereqNum = 0;
    }
  }
  
  else if(state == PROMPT2){
    if(millis() - waiting_timer > 60000){
      state = RING;
      screen.clear_body();
      buzzer.start();
      return;
    }
    sprintf(screen.body_str, "Are you taking the\nmedicine now?\n \n  > A for Yes\n  > B for No\n");
    if(state1){
      waiting_timer = millis();
      state = SUCCESS;
      screen.clear_body();
    }
    else if(state2){
      waiting_timer = millis();
      state = RING;
      screen.clear_body();
      buzzer.start();
      prereqNum = 0;
    }
  }
  
  else if(state == SUCCESS){
    sprintf(screen.body_str, ":)");
    currentMedicine.dosage = -1;
    secondary_timer = millis();
    tertiary_timer = millis();
    state = SUCCESS_WAIT;
    take_medicine(currentMedicine.med_name, "timely");
    // added to clear screen
    screen.clear_body();
    currentMedicine = getNext(now.forward(-30 * ONE_MINUTE));
  }
  
  else if(state == SUCCESS_WAIT){
    if(millis() - tertiary_timer > 5000)
      state = IDLE;
      screen.clear_body();
      sprintf(screen.body_str, "");
  }
}

/*
 * Do a POST request that the user is taking the medicine.
 * Status is either "timely", "early", or "late"
 */
 
void take_medicine(char* med_name, char* status) {
  char body[100];
  sprintf(body, "user_name=%s&update=daily_take&med_name=%s&status=%s", user.name, med_name, status);
  int body_len = strlen(body);
  sprintf(request_buffer, "POST http://608dev-2.net/sandbox/sc/team103/dayMax/update_info.py HTTP/1.1\r\n");
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(request_buffer, "\r\n"); //new line from header to body
  strcat(request_buffer, body); //body
  strcat(request_buffer, "\r\n"); //header
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}

/*
 * Do a POST request that the user is updating their health conditions
 * Update field is cond
 */

void update_condition(char* heart, char* respiration, char* systolic, char* diastolic, char* blood_sugar) {
  char body[300];
  sprintf(body, "user_name=%s&update=cond&heart_rate=%s&respiration=%s&systolic=%s&diastolic=%s&blood_sugar=%s", user.name, heart, respiration, systolic, diastolic, blood_sugar);
  int body_len = strlen(body);
  sprintf(request_buffer, "POST http://608dev-2.net/sandbox/sc/team103/dayMax/update_info.py HTTP/1.1\r\n");
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(request_buffer, "\r\n"); //new line from header to body
  strcat(request_buffer, body); //body
  strcat(request_buffer, "\r\n"); //header
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
}
