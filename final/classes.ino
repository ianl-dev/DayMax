
int Time::cmp(Time other){
  return 3600000 * (other.hour - hour) + 60000 * (other.minute - minute) + 1000 * (other.second - second);
}

Time Time::forward(int timeForward){
  int tot = 3600000 * hour + 60000 * minute + 1000 * second + timeForward;
  tot /= 1000;
  return {(tot / (60 * 60)) % 24, (tot / 60) % 60, tot % 60};
}

void Time::toString(char* buffer){
  char meh[] = "2";
  if(hour < 10) meh[0] = ' ';
  else if(hour < 20) meh[0] = '1';
  sprintf(buffer, "%s%d:%d%d", meh, hour % 10, minute / 10, minute % 10);
}

void Time::toShortString(char* buffer){
  if(hour < 10)
    sprintf(buffer, "%d:%d%d", hour % 10, minute / 10, minute % 10);
  else
    sprintf(buffer, "%d%d:%d%d", hour / 10, hour % 10, minute / 10, minute % 10);
}
/*
 * LCD function: Initialization
 */
LCD::LCD(){
  tft.init();
  tft.setRotation(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
}
void LCD::write_body(char* stuff){
  sprintf(body_str, stuff);
}
void LCD::write_header(char* stuff){
  sprintf(header_str, stuff);
}
void LCD::write_time(Time stuff){
  displayedTime = stuff;
}
void LCD::clear_body(){
  tft.fillRect(0, 21, 130, 140, TFT_BLACK);
  sprintf(body_str, "");
}
/*
 * LCD function: display saved string in str
 */
void LCD::display_body(){
  //format lcd_buffer and display
  strcpy(ffs, body_str);
  char* tok_buffer = strtok(ffs, "\n");
  int cur_height = startingBodyHeight - curScrollAmount;
  int count = 0;
  while(tok_buffer != NULL && cur_height < endingBodyHeight){
    if(count == frozenLines - 1 && frozenLines != 0){
      sprintf(temp_buffer, tok_buffer);
    }
    if(cur_height >= startingBodyHeight - 8 + frozenLines * lineHeight){
      sprintf(draw_buffer, "                      ");
      strncpy(draw_buffer, tok_buffer, strlen(tok_buffer));
      
      tft.drawLine(0, cur_height - 4, 130, cur_height - 4, TFT_BLACK);
      tft.drawLine(0, cur_height - 3, 130, cur_height - 3, TFT_BLACK);
      tft.drawLine(0, cur_height - 2, 130, cur_height - 2, TFT_BLACK);
      tft.drawLine(0, cur_height - 1, 130, cur_height - 1, TFT_BLACK);
      tft.drawLine(0, cur_height + 8, 130, cur_height + 8, TFT_BLACK);
      tft.drawLine(0, cur_height + 9, 130, cur_height + 9, TFT_BLACK);
      tft.drawLine(0, cur_height + 10, 130, cur_height + 10, TFT_BLACK);
      tft.drawString(draw_buffer, 5, cur_height, 1);

      if(count == linePointer - 1 && showPointer){
        int dif = millis() - pointerUpdate;
        if(dif % 1000 < 500)
          sprintf(draw_buffer, " >");
        else
          sprintf(draw_buffer, "  ");
        tft.drawString(draw_buffer, 5, startingBodyHeight + (curPointer - 1) * lineHeight, 1);
      }
      
      if(cur_height < startingBodyHeight){
        tft.drawLine(0, 18, 130, 18, TFT_GREEN);
        tft.drawLine(0, 19, 130, 19, TFT_BLACK);
        tft.drawLine(0, 20, 130, 20, TFT_GREEN);
        tft.drawLine(0, 21, 130, 21, TFT_BLACK);
        tft.drawLine(0, 22, 130, 22, TFT_BLACK);
        tft.drawLine(0, 23, 130, 23, TFT_BLACK);
        tft.drawLine(0, 24, 130, 24, TFT_BLACK);
      }
      if(cur_height < startingBodyHeight + (frozenLines + 1) * lineHeight && frozenLines != 0){
        sprintf(draw_buffer, "                      ");
        strncpy(draw_buffer, temp_buffer, strlen(temp_buffer));
        tft.drawString(draw_buffer, 5, startingBodyHeight + (frozenLines - 1) * lineHeight, 1);
      }
    }
    count++;
    cur_height += lineHeight;
    tok_buffer = strtok(NULL, "\n");
  }
  for(int i = 150; i < 160; i++){
    tft.drawLine(0, i, 130, i, TFT_BLACK);
  }
  strcpy(ffs, body_str);
  tok_buffer = strtok(ffs, "\n");
  cur_height = startingBodyHeight;
  while(tok_buffer != NULL && cur_height < startingBodyHeight + frozenLines * lineHeight){
    if(cur_height >= startingBodyHeight - 8){
      sprintf(draw_buffer, "                      ");
      strncpy(draw_buffer, tok_buffer, strlen(tok_buffer));
      tft.drawLine(0, cur_height - 4, 130, cur_height - 4, TFT_BLACK);
      tft.drawLine(0, cur_height - 3, 130, cur_height - 3, TFT_BLACK);
      tft.drawLine(0, cur_height - 2, 130, cur_height - 2, TFT_BLACK);
      tft.drawLine(0, cur_height - 1, 130, cur_height - 1, TFT_BLACK);
      tft.drawLine(0, cur_height + 8, 130, cur_height + 8, TFT_BLACK);
      tft.drawLine(0, cur_height + 9, 130, cur_height + 9, TFT_BLACK);
      tft.drawLine(0, cur_height + 10, 130, cur_height + 10, TFT_BLACK);
      tft.drawString(draw_buffer, 5, cur_height, 1);
      if(cur_height < startingBodyHeight){
        tft.drawLine(0, 18, 130, 18, TFT_GREEN);
        tft.drawLine(0, 19, 130, 19, TFT_BLACK);
        tft.drawLine(0, 20, 130, 20, TFT_GREEN);
        tft.drawLine(0, 21, 130, 21, TFT_BLACK);
        tft.drawLine(0, 22, 130, 22, TFT_BLACK);
        tft.drawLine(0, 23, 130, 23, TFT_BLACK);
        tft.drawLine(0, 24, 130, 24, TFT_BLACK);
      }
    }
    cur_height += lineHeight;
    tok_buffer = strtok(NULL, "\n");
  }
  /*
  while(cur_height < endingBodyHeight){
    sprintf(draw_buffer, "                      ");
    tft.drawString(draw_buffer, 0, cur_height, 1);
    cur_height += lineHeight;
  }
  */
}
void LCD::display_header(){
  sprintf(draw_buffer, header_str);
  tft.drawString(draw_buffer, 5, 6, 1);
}
void LCD::display_time(){
  displayedTime.toString(draw_buffer);
  tft.drawString(draw_buffer, 90, 6, 1);
}
void LCD::setPointer(bool pointerSetting){
  showPointer = pointerSetting;
  pointerUpdate = millis();
}
void LCD::scrollLines(int dir){
  bodyLines = 1;
  for(int i = 0; i < strlen(body_str); i++){
    if(body_str[i] == '\n')
      bodyLines++;
  }
  curPointer += dir;
  curPointer = max(curPointer, frozenLines + 1);
  curPointer = min(curPointer, 9);
  curPointer = min(curPointer, bodyLines);
  linePointer += dir;
  linePointer = min(linePointer, bodyLines);
  linePointer = max(linePointer, frozenLines + 1);
  scrollAmount = lineHeight * (linePointer - curPointer);
  //scrollAmount += (dir * lineHeight);
  //scrollAmount = min(scrollAmount, bodyLines * lineHeight - (endingBodyHeight - startingBodyHeight));
  //scrollAmount = max(scrollAmount, 0);
}
void LCD::resetLines(int newFrozen){
  frozenLines = newFrozen;
  scrollAmount = 0;
  curScrollAmount = 0;
  linePointer = frozenLines + 1;
  curPointer = frozenLines + 1;
  pointerUpdate = millis();
}
void LCD::display() {
  //tft.fillScreen(TFT_BLACK);
  int dif = millis() - lastUpdate;
  dif = max(dif, 0);
  lastUpdate = millis();
  if(curScrollAmount < scrollAmount){
    curScrollExact = curScrollExact + 0.07 * dif;
    if(curScrollExact > scrollAmount)
      curScrollExact = scrollAmount;
    curScrollAmount = (int)(curScrollExact + 0.5);
  }
  else{
    curScrollExact = curScrollExact - 0.07 * dif;
    if(curScrollExact < scrollAmount)
      curScrollExact = scrollAmount;
    curScrollAmount = (int)(curScrollExact + 0.5);
  }
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  display_body();
  
  //tft.fillRect(0, 0, 150, 20, TFT_BLACK);
  //sprintf(draw_buffer, "                      ");
  //tft.drawString(draw_buffer, 0, 0, 1);
  //tft.drawString(draw_buffer, 0, 5, 1);
  //tft.drawString(draw_buffer, 0, 10, 1);
  display_header();
  display_time();
  tft.drawLine(0, 18, 130, 18, TFT_GREEN);
  tft.drawLine(0, 19, 130, 19, TFT_BLACK);
  tft.drawLine(0, 20, 130, 20, TFT_GREEN);
  tft.drawLine(0, 21, 130, 21, TFT_BLACK);
  tft.drawLine(0, 22, 130, 22, TFT_BLACK);
  tft.drawLine(0, 23, 130, 23, TFT_BLACK);
  tft.drawLine(0, 24, 130, 24, TFT_BLACK);
}

/*
 * Alarm function: initialization
 */

// Take on me music
#define A5  880
#define B4  494
#define B5  988
#define D5  587
#define E5  659
#define FS5 740
#define GS5 831

int melody[] = {
  FS5, FS5, D5, B4, B4, E5, 
  E5, E5, GS5, GS5, A5, B5,
  A5, A5, A5, E5, D5, FS5, 
  FS5, FS5, E5, E5, FS5, E5
};

int rhythm[] = {
  8, 8, 8, 4, 4, 4, 
  4, 5, 8, 8, 8, 8, 
  8, 8, 8, 4, 4, 4, 
  4, 5, 8, 8, 8, 8
};

/*
 * Alarm function: turn on alarm
 */

Alarm::Alarm(){
    ledcSetup(0,5000,12); // 0 for channel, 5000 for frequency, 12 for resolution
    ledcAttachPin(12,0);
//  ledcSetup(0, 60, 8);
//  ledcAttachPin(redPin, 0);
//  ledcSetup(1, 60, 8);
//  ledcAttachPin(greenPin, 1);
//  ledcSetup(2, 60, 8);
//  ledcAttachPin(bluePin, 2);
}

void Alarm::start(){
  startTime = millis();
  int song = sizeof(melody)/sizeof(melody[0]);
  for (int note = 0; note < song; note++){
    // determine the duration of the notes that the computer understands
    // divide 1200 by the value, so the first note lasts for 1200/8 milliseconds (1200 to be slower)
    int duration = 1200/rhythm[note];
    ledcWriteTone(0,melody[note]);
    // pause between notes
    int pause = duration * 1.3;
    delay(pause);
    ledcWriteTone(0,0);
    // stop the tone
  }
//  blare = true;
}
/*
 * Alarm function: turn off LED
 */
void Alarm::stop(){
  ledcWriteTone(0,0); 
//  redVal = 0;
//  greenVal = 0;
//  blueVal = 0;
//  ledcWrite(0, redVal);
//  ledcWrite(1, greenVal);
//  ledcWrite(2, blueVal);
//  blare = false;
}
/*
 * Alarm function: if alarm is on, turn it off, and vice versa
 */
//void Alarm::flip(){
//  if(blare)
//    stop();
//  else
//    start();
//}
/*
 * Alarm function: 
 */
//void Alarm::apply(){
//  if(!blare){
//    redVal = 0;
//    greenVal = 0;
//    blueVal = 0;
//    ledcWrite(0, redVal);
//    ledcWrite(1, greenVal);
//    ledcWrite(2, blueVal);
//    return;
//  }
//  uint32_t gap = millis() - startTime;
//  //if(gap > duration)
//  //  stop();
//  float diff = (1.0*(gap%period))/period;
//  redVal = (int)(fabs(diff-0.5) * 511);
//  greenVal = (int)((0.5-fabs(diff-0.5))*511);
//  blueVal = 0;
//  ledcWrite(0, redVal);
//  ledcWrite(1, greenVal);
//  ledcWrite(2, blueVal);
//}
//bool Alarm::isBlaring(){
//  return blare;
//}

Button::Button(int p) {
  flag = 0;  
  state = 0;
  pin = p;
  t_of_state_2 = millis(); //init
  t_of_button_change = millis(); //init
  debounce_time = 10;
  long_press_time = 1000;
  button_pressed = 0;
}
void Button::read() {
  uint8_t button_state = digitalRead(pin);  
  button_pressed = !button_state;
}
int Button::update() {
  read();
  flag = 0;
  if (state==0) {
    if (button_pressed) {
      state = 1;
      t_of_button_change = millis();
    }
  } else if (state==1) {
    if (!button_pressed) {
      state = 0;
      t_of_button_change = millis();
    }
    else {
      if (millis() - t_of_button_change >= debounce_time) {
        state = 2;
        t_of_state_2 = millis();
      }
    }
  } else if (state==2) {
    if (!button_pressed) {
      state = 4;
      t_of_button_change = millis();
    }
    else {
      if (millis() - t_of_state_2 >= long_press_time )
        state = 3;
    }
  } else if (state==3) {
    if (!button_pressed) {
      state = 4;
      t_of_button_change = millis();
    }
  } else if (state==4) {        
    if (button_pressed) {
      t_of_button_change = millis();
      if (millis() - t_of_state_2 >= long_press_time)
        state = 3;
      else
        state = 2;
    }
    else {
      if (millis() - t_of_button_change >= debounce_time) {
        state = 0;
        if (millis() - t_of_state_2 >= long_press_time)
          flag = 2;
        else
          flag = 1;
      }
    }
  }
  return flag;
}

void Medication::reset(){
  dosage = 0;
  timeNum = 0;
  todayNum = 0;
  prereqMeal = false;
  prereqTemp = false;
}

void Medication::assignInfo(char* p_med){
  reset();
  char *saveptr2, *saveptr3;
  //strcpy(p_med, infoBuffer);
  char *p = strtok_r(p_med, ",", &saveptr2);
  int cnt = 0, tNum = -1;
  timeNum = 0;
  // 0 -> name, 1 -> dosage, 2 -> number of time, 3 -> time taken, 4 -> F/T, 5 (if T) -> before/after
  while (p != NULL) { // process parts of a medicine
    if (cnt == 0) sprintf(med_name, p);
    else if (cnt == 1) dosage = atoi(p);
    else if (cnt == 2) tNum = atoi(p);
    else if (cnt >= 3 && cnt < 3 + tNum) {
      // TODO: change this when multiple time is implemented
      char* ptime = strtok_r(p, ":", &saveptr3);
      Time processed_time = {0, 0, 0};
      processed_time.hour = atoi(ptime);
      ptime = strtok_r(NULL, ":", &saveptr3);
      processed_time.minute = atoi(ptime);
      addTime(processed_time);
    }
    else if (cnt == 3 + tNum && tNum != -1) {
      if (strcmp (p,"F") == 0) prereqMeal = false;
      else this->prereqMeal = true;
    }
    else if (prereqMeal == true && cnt == 4 + tNum && tNum != -1) {
      if (strcmp (p,"before") == 0) prereqMealDetail = 'B'; // before
      else prereqMealDetail = 'A'; // after
    }
    else{
      Serial.println("Error in Medication::assignInfo?");
    }
    cnt++;
    p = strtok_r(NULL, ",", &saveptr2);
  }
}

int Medication::getPrereqPrompt(int cur, char* prompt){
  if(cur == 0){
    if(!prereqMeal)
      return getPrereqPrompt(cur + 1, prompt);
    char prereqStr[20] = "before";
    if(prereqMealDetail == 'D')
      sprintf(prereqStr, "during");
    if(prereqMealDetail == 'A')
      sprintf(prereqStr, "after");
    sprintf(prompt, "Did you remember to\ntake %s\n%s your meal?\n \n  > A for Yes\n  > B for No\n", med_name, prereqStr);
    return cur + 1;
  }
  if(cur == 1){
    return -1;
  }
}

void Medication::addTime(Time newTime){
  times[timeNum] = newTime;
  timeNum++;
}

bool Medication::isTaken() {
  sprintf(request_buffer, "GET /sandbox/sc/team103/test/get_daily_take_info.py?user_name=%s&comm_type=taken&med_name=%s HTTP/1.1\r\n", user.name, med_name);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  if (strcmp(response_buffer, "True\n") == 0) return true;
  else return false;
}

void Clock::updateTime(){
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/get_current_time.py?user_name=%s HTTP/1.1\r\n", user.name);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  counter = millis();

  char* ptime = strtok(response_buffer, ":");
  lastTime.hour = atoi(ptime);
  ptime = strtok(NULL, ":");
  lastTime.minute = atoi(ptime);
  ptime = strtok(NULL, ":");
  lastTime.second = atoi(ptime);
  //lastTime = lastTime.forward(60 * 60 * 1000 * 18);
}

Time Clock::getTime(){
  if(millis() - counter > 60 * ONE_MINUTE)
    updateTime();
  int diff = millis() - counter;
  diff /= 1000;
  Time ret = lastTime;
  ret.hour += (diff / (60 * 60));
  ret.minute += ((diff / 60) % 60);
  ret.second += (diff % 60);
  if(ret.second >= 60){
    ret.second -= 60;
    ret.minute++;
  }
  if(ret.minute >= 60){
    ret.minute -= 60;
    ret.hour++;
  }
  ret.hour %= 24;
  return ret;
}
