/*
 * This file is for email notification functionality. 
 * If you want to remove the feature, comment this whole file and
 * the email-related code in "final" file.
 */
#include "ESP32_MailClient.h"

// SMPT settings for email notifications
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
SMTPData smtpData;

// email notification sender account
#define emailSenderAccount "daymax103@gmail.com"
#define emailSenderPassword "team103103"

/* 
 *  email timing constants: every 30 second send notification to user, 
 *  after 2 minutes send to both user and provider
*/
const int user_notif_timing = 60000;
const int provider_notif_timing = 120000;

void SMTP_setup() {
  // SMTP setup
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  smtpData.setSender("DayMax-daily", emailSenderAccount);
  smtpData.setPriority("High");
}

void daily_notif_user(char* med_name, int dosage) {
    // send notification to user
    SMTP_setup();
    // set email subject
    char email_subject[100];
    sprintf(email_subject, "[DayMax] Drink your medicine - %s", med_name);
    smtpData.setSubject(email_subject);

    // set email body
    char email_body[200];
    sprintf(email_body, "Hi %s! It's time to drink %d of %s. Read the instruction on DayMax device for further steps.", user.name, dosage, med_name);
    smtpData.setMessage(email_body, false);
    
    get_basic_info(); // call GET basic info just in case the info is updated
    smtpData.addRecipient(user.email);
    smtpData.setDebug(true);
    if (!MailClient.sendMail(smtpData)) {
      smtpData.setDebug(true);
      Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    }
    smtpData.empty();
}

void weekly_notif_provider(int provider_timer) {
  unsigned long time_diff_provider = millis() - provider_timer;
  if (time_diff_provider > provider_notif_timing) {
    // send notification to provider
    SMTP_setup();
    smtpData.setSubject("[DayMax] Medicine Taking Status from Stacia");
    smtpData.setMessage("During this week, Stacia taken all her medicine on time!", false);

    get_basic_info();
    smtpData.addRecipient(user.providerEmail);
    if (!MailClient.sendMail(smtpData)) {
      smtpData.setDebug(true);
      Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    }
    smtpData.empty();
    provider_timer = millis();
  }
}
