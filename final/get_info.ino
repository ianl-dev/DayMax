void get_basic_info() {
    sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/get_basic_info.py?name=%s HTTP/1.1\r\n", user.name);
    strcat(request_buffer, "Host: 608dev-2.net\r\n");
    strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
    strcat(request_buffer, "\r\n");
    strcat(request_buffer, "\r\n");
    Serial.println(request_buffer);
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

    char no_user[] = "No user found";
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
    }
}

void get_med_info() {
  // test using dayMax on server
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/get_med_info.py?name=%s HTTP/1.1\r\n", user.name);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

  char no_med[] = "";

  // CASE 1: Medications found
  if (strcmp (response_buffer,no_med) != 0) { // if medicine is found
    medicineNum = 0;
    char *p_med;
    char *saveptr1;
    p_med = strtok_r(response_buffer, "-", &saveptr1);
    while (p_med != NULL && strcmp(p_med, "\n") != 0) { // for every medicine
      medicine[medicineNum].assignInfo(p_med);
      medicineNum++;
      Serial.printf("%d %s %d %d %d\n", medicineNum, medicine[medicineNum-1].med_name, medicine[medicineNum-1].dosage, medicine[medicineNum-1].times[0].hour, medicine[medicineNum-1].times[0].minute);
      strcat(late_med, medicine[medicineNum-1].med_name);
      strcat(late_med, "-");
      p_med = strtok_r(NULL, "-", &saveptr1);
    }
  }
}

Medication getNext(Time now){
  Medication ret;
  ret.dosage = -1;
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/alarm.py?name=%s&action=next&time=%d:%d HTTP/1.1\r\n", user.name, now.hour, now.minute);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
  if(strlen(response_buffer) > 2){
    char *p_med;
    char *saveptr1;
    p_med = strtok_r(response_buffer, "-", &saveptr1);
    ret.assignInfo(p_med);
  }
  Serial.printf("TIME %d:%d:%d\n", ret.times[0].hour, ret.times[0].minute, ret.times[0].second);
  return ret;
}

// Missed 
void getMissed(Time now){
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/alarm.py?name=%s&action=status&type=missed&time=%d:%d HTTP/1.1\r\n", user.name, now.hour, now.minute);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
  //sprintf(response_buffer, "10:20 MedA-10:30 MedB-10:40 MedC-10:50 MedD-11:00 MedE-11:10 MedF-11:20 MedG-11:30 MedH-11:40 MedI-11:50 MedJ-");
  char* tok_buffer = strtok(response_buffer, "-");
  sprintf(list_buffer, "");
  while(tok_buffer != NULL && strlen(tok_buffer) > 1){
    strcat(list_buffer, "\n   ");
    strcat(list_buffer, tok_buffer);
    tok_buffer = strtok(NULL, "-");
  }
}


// Missed 2
int getMissedNum(Time now){
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/alarm.py?name=%s&action=status&type=missed&time=%d:%d HTTP/1.1\r\n", user.name, now.hour, now.minute);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
  if(strlen(response_buffer) <= 2)
    return 0;
  int ret = 0;
  for(int i = 0; i < strlen(response_buffer); i++){
    if(response_buffer[i] == '-')
      ret++;
  }
  return ret;
}

// Met
void getMet(Time now){
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/alarm.py?name=%s&action=status&type=taken&time=%d:%d HTTP/1.1\r\n", user.name, now.hour, now.minute);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
  //sprintf(response_buffer, "10:10 MedA-");
  char* tok_buffer = strtok(response_buffer, "-");
  sprintf(list_buffer, "");
  while(tok_buffer != NULL && strlen(tok_buffer) > 1){
    strcat(list_buffer, "\n   ");
    strcat(list_buffer, tok_buffer);
    tok_buffer = strtok(NULL, "-");
  }
}

// Upcoming
void getUpcoming(Time now){
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/alarm.py?name=%s&action=status&type=upcoming&time=%d:%d HTTP/1.1\r\n", user.name, now.hour, now.minute);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
  //sprintf(response_buffer, "15:20 MedA-15:30 MedB-15:40 MedC-");
  char* tok_buffer = strtok(response_buffer, "-");
  sprintf(list_buffer, "");
  while(tok_buffer != NULL && strlen(tok_buffer) > 1){
    strcat(list_buffer, "\n   ");
    strcat(list_buffer, tok_buffer);
    tok_buffer = strtok(NULL, "-");
  }
}

// Pending 1
void getPending(Time now){
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/alarm.py?name=%s&action=status&type=pending&time=%d:%d HTTP/1.1\r\n", user.name, now.hour, now.minute);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
  //sprintf(response_buffer, "-");
  char* tok_buffer = strtok(response_buffer, "-");
  sprintf(list_buffer, "");
  while(tok_buffer != NULL && strlen(tok_buffer) > 1){
    strcat(list_buffer, "\n   ");
    strcat(list_buffer, tok_buffer);
    tok_buffer = strtok(NULL, "-");
  }
}


// Pending 2
int getPendingNum(Time now){
  sprintf(request_buffer, "GET /sandbox/sc/team103/dayMax/alarm.py?name=%s&action=status&type=pending&time=%d:%d HTTP/1.1\r\n", user.name, now.hour, now.minute);
  strcat(request_buffer, "Host: 608dev-2.net\r\n");
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  strcat(request_buffer, "\r\n");
  strcat(request_buffer, "\r\n");
  Serial.println(request_buffer);
  do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  Serial.println(response_buffer);
  if(strlen(response_buffer) <= 2)
    return 0;
  int ret = 0;
  for(int i = 0; i < strlen(response_buffer); i++){
    if(response_buffer[i] == '-')
      ret++;
  }
  return ret;
}


// Report
void get_late_med(Time now){
  sprintf(response_buffer, late_med);
  char* tok_buffer = strtok(response_buffer, "-");
  sprintf(list_buffer, "");
  while(tok_buffer != NULL && strlen(tok_buffer) > 0){
    strcat(list_buffer, "\n   ");
    strcat(list_buffer, tok_buffer);
    tok_buffer = strtok(NULL, "-");
  } 
}

// Measure
void get_angle(float* x, float* y) {
  imu.readAccelData(imu.accelCount);
  *x = imu.accelCount[0] * imu.aRes;
  *y = imu.accelCount[1] * imu.aRes;
}
