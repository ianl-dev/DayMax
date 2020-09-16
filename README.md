# DayMax
Inspired by BayMax, DayMax is an arduino-based reminder system that improves medical non-adherence

<img src="https://www.awn.com/sites/default/files/styles/original/public/image/attached/1023010-big-hero-6-leftfullcomp.0025-1200.jpg?itok=W9pdebjv" width="1000" height="350">

# Installation Note
Before using DayMax, some settings for the email notification service must be configured.
1. Include ESP32 Mail Client library. In the Arduino IDE go to Sketch > Include Library > Manage Libraries. Search for "ESP32 Mail Client" by Mobizt, and install the library.
2. Close and restart the Arduino IDE.
3. If when compiling/uploading, you get an error of "#error: architecture or board not supported", go to the Arduino libraries folder in your local computer, then move/delete the entire "SD" folder from the libraries folder.  

If you want to remove all the email notification, comment everything on the email_notif.ino file and the code in final.ino that calls the email function (search "email").
