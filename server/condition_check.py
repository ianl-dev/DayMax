import sqlite3
import datetime
user_info_db = '__HOME__/dayMax/user_info.db'

# libraries for email
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
MY_ADDRESS = 'daymax103@gmail.com'
PASSWORD = 'team103103'

"""
This file contains helper functions that calculate the user's condition and report it accordingly.

The available GET command:
1) Get user's weekly condition: list of all user condition input within last 7 days
   Method: GET with parameter user_name and command='weekly'

"""

# Constants related to range of safe condition
heart_rate_low = 60
heart_rate_high = 100
respiration_rate_low = 12
respiration_rate_high = 20
systolic_pressure_low = 90
systolic_pressure_high = 120
diastolic_pressure_low = 60
diastolic_pressure_high = 80
blood_sugar_low = 50
blood_sugar_high = 100

"""
This helper function is to check whether the average of the user's condition within a certain number of days
are outside the safe range. This can be used for condition that may fluctuate a lot between different days but
but are actually safe.
Parameter:
user: patient's username
num_day: number of consecutive days in which the condition will be averaged
condition: the specific condition that will checked. 
           Currently it should be between 'heart_rate', 'respiration_rate', 'systolic_pressure', 'diastolic_pressure', 'blood_sugar'
Return: 'safe x' or 'unsafe x' where x is the average of the condition
"""
def average_over_days(user, num_day, condition, with_email):
    conn = sqlite3.connect(user_info_db)
    c = conn.cursor()
    num_days = int(num_day)
    days_ago = datetime.datetime.now()- datetime.timedelta(days = num_days) 
    conds = c.execute('''SELECT * FROM condition_table WHERE user_name = ? AND timing > ?;''',(user,days_ago)).fetchall()
    conn.commit()
    c.close()
    conn.close()
    count = len(conds)
    if count == 0:
        return 'safe 0'
    total = 0
    for cond in conds:
        if condition == 'heart_rate':
            total += cond[1]
        elif condition == 'respiration_rate':
            total += cond[2]
        elif condition == 'systolic_pressure':
            total += cond[3]
        elif condition == 'diastolic_pressure':
            total += cond[4]
        elif condition == 'blood_sugar':
            total += cond[5]
    avg = int(round(total / count))

    # Checking the average w/ the constants
    safe_cond = condition == 'heart_rate' and heart_rate_low <= avg and avg <= heart_rate_high
    safe_cond = safe_cond or (condition == 'respiration_rate' and respiration_rate_low <= avg and avg <= respiration_rate_high)
    safe_cond = safe_cond or (condition == 'systolic_pressure' and systolic_pressure_low <= avg and avg <= systolic_pressure_high)
    safe_cond = safe_cond or (condition == 'diastolic_pressure' and diastolic_pressure_low <= avg and avg <= diastolic_pressure_high)
    safe_cond = safe_cond or (condition == 'blood_sugar' and blood_sugar_low <= avg and avg <= blood_sugar_high)
    if safe_cond:
        return 'safe ' + str(avg)
    else: 
        if with_email:
            email_body = "Your patient's - " + user + " average " + condition + " is not within the safe range in the last " + str(num_day) + "day(s). The average is " + str(avg) + ". Please check in with your patient."
            send_email(user, email_body)
        return 'unsafe ' + str(avg)

"""
This helper function is to check whether the average of the user's condition within a certain number of times
are outside the safe range.
Parameter:
user: patient's username
num_time: number of consecutive times in which the condition will be averaged
condition: the specific condition that will checked. 
           Currently it should be between 'heart_rate', 'respiration_rate', 'systolic_pressure', 'diastolic_pressure', 'blood_sugar'
Return: 'safe x' or 'unsafe x' where x is the average of the condition
"""
def average_over_times(user, num_time, condition, with_email=False):
    conn = sqlite3.connect(user_info_db)
    c = conn.cursor()
    num_times = int(num_time)
    conds = c.execute('''SELECT * FROM condition_table WHERE user_name = ? ORDER BY timing DESC LIMIT ?;''',(user,num_times)).fetchall()
    conn.commit()
    c.close()
    conn.close()
    count = len(conds)
    if count == 0:
        return 'safe 0'
    total = 0
    for cond in conds:
        if condition == 'heart_rate':
            total += cond[1]
        elif condition == 'respiration_rate':
            total += cond[2]
        elif condition == 'systolic_pressure':
            total += cond[3]
        elif condition == 'diastolic_pressure':
            total += cond[4]
        elif condition == 'blood_sugar':
            total += cond[5]
    avg = int(round(total / count))

    # Checking the average w/ the constants
    safe_cond = condition == 'heart_rate' and heart_rate_low <= avg and avg <= heart_rate_high
    safe_cond = safe_cond or (condition == 'respiration_rate' and respiration_rate_low <= avg and avg <= respiration_rate_high)
    safe_cond = safe_cond or (condition == 'systolic_pressure' and systolic_pressure_low <= avg and avg <= systolic_pressure_high)
    safe_cond = safe_cond or (condition == 'diastolic_pressure' and diastolic_pressure_low <= avg and avg <= diastolic_pressure_high)
    safe_cond = safe_cond or (condition == 'blood_sugar' and blood_sugar_low <= avg and avg <= blood_sugar_high)
    if safe_cond:
        return 'safe ' + str(avg)
    else: 
        if with_email:
            email_body = "Your patient's - " + user + " average " + condition + " is not within the safe range in the last " + str(num_times) + " time(s). The average is " + str(avg) + ". Please check in with your patient."
            send_email(user, email_body)
        return 'unsafe ' + str(avg)

def consistent_over_times(user, num_time, condition, with_email=False):
    conn = sqlite3.connect(user_info_db)
    c = conn.cursor()
    num_times = int(num_time)
    conds = c.execute('''SELECT * FROM condition_table WHERE user_name = ? ORDER BY timing DESC LIMIT ?;''',(user,num_times)).fetchall()
    conn.commit()
    c.close()
    conn.close()
    count = len(conds)
    if count == 0:
        return 'safe 0'
    total = 0
    is_safe = False
    datas = ''
    for cond in conds:
        if condition == 'heart_rate':
            datas = datas + str(cond[1])
        elif condition == 'respiration_rate':
            datas = datas + str(cond[2])
        elif condition == 'systolic_pressure':
            datas = datas + str(cond[3])
        elif condition == 'diastolic_pressure':
            datas = datas + str(cond[4])
        elif condition == 'blood_sugar':
            datas = datas + str(cond[5])
        if condition == 'heart_rate' and heart_rate_low <= cond[1] and cond[1] <= heart_rate_high:
            is_safe = True
            break
        elif condition == 'respiration_rate' and respiration_rate_low <= cond[2] and cond[2] <= respiration_rate_high:
            is_safe = True
            break
        elif condition == 'systolic_pressure' and systolic_pressure_low <= cond[3] and cond[3] <= systolic_pressure_high:
            is_safe = True
            break
        elif condition == 'diastolic_pressure' and diastolic_pressure_low <= cond[4] and cond[4] <= diastolic_pressure_high:
            is_safe = True
            break
        elif condition == 'blood_sugar' and blood_sugar_low <= cond[5] and cond[5] <= blood_sugar_high:
            is_safe = True
            break
    if is_safe:
        return 'safe'
    else: 
        if with_email:
            email_body = "Your patient's - " + user + " " + condition + " is not within the safe range in the last " + str(num_times) + " time(s). They are " + str(datas) + ". Please check in with your patient."
            send_email(user, email_body)
        return 'unsafe ' + datas

def send_email(user, body):
    # Get email from database
    conn = sqlite3.connect(user_info_db)
    c = conn.cursor()
    users = c.execute('''SELECT * FROM user_table WHERE user_name = ?;''',(user,)).fetchone()
    email = users[7]
    conn.commit()
    c.close()
    conn.close()

    # set up the SMTP server
    s = smtplib.SMTP(host='smtp.gmail.com', port=587)
    s.starttls()
    s.login(MY_ADDRESS, PASSWORD)
    msg = MIMEMultipart()

    msg['From']=MY_ADDRESS
    msg['To']=email
    msg['Subject']="[Patient Alert] Review Extenuating Condition"

    msg.attach(MIMEText(body, 'plain'))
    s.send_message(msg)
    del msg
    s.quit()


def request_handler(request):
    if request['method']=='GET':
        try:
            user = request['values']['user_name']
        except:
            return 'Username does not exist'
        try:
            command = request['values']['command']
        except:
            return 'Command does not exist'
        if command == 'weekly':
            conn = sqlite3.connect(user_info_db)
            c = conn.cursor()
            conds = c.execute('''SELECT * FROM condition_table WHERE user_name = ? AND timing > (SELECT DATETIME('now', '-7 day'));''',(user,)).fetchall()
            conn.commit()
            c.close()
            conn.close()
            return conds
        elif command == 'average_over_days':
            try:
                num_day = request['values']['num_day']
                condition = request['values']['condition']
            except:
                return 'Required parameters are num_day and condition'
            try:
                with_email = request['values']['email']
            except:
                with_email = False
            return average_over_days(user, num_day, condition, with_email)
        elif command == 'average_over_times':
            try:
                num_time = request['values']['num_time']
                condition = request['values']['condition']
            except:
                return 'Required parameters are num_time and condition'
            try:
                with_email = request['values']['email']
            except:
                with_email = False
            return average_over_times(user, num_time, condition, with_email)
        elif command == 'consistent_over_times':
            try:
                num_time = request['values']['num_time']
                condition = request['values']['condition']
            except:
                return 'Required parameters are num_time and condition'
            try:
                with_email = bool(request['values']['email'])
            except:
                with_email = False
            return consistent_over_times(user, num_time, condition, with_email)