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
Purpose: get information that are related to the user's daily medicine taking status

Type of GET command handled:
1. GET all status of the last 7 days
   include parameter user_name and comm_type='weekly'
2. GET status whether a particular medicine is taken in the last 5 minutes
   include parameter user_name, comm_type='taken', med_name
"""

def request_handler(request):
    if request['method']=='GET':
        conn = sqlite3.connect(user_info_db)
        c = conn.cursor()
        try:
            user = request['values']['user_name']
            comm_type = request['values']['comm_type']
        except:
            return 'Missing parameters from GET request.'
        if comm_type == 'weekly':
            takes = c.execute('''SELECT * FROM daily_take_table WHERE user_name = ? AND timing > (SELECT DATETIME('now', '-7 day'));''',(user,)).fetchall()
            conn.commit()
            c.close()
            conn.close()
            return takes
        elif comm_type == 'taken':
            try:
                med = request['values']['med_name']
            except:
                return 'Missing parameters from GET request.'
            takes = c.execute('''SELECT * FROM daily_take_table WHERE user_name = ? AND med_name = ? AND timing > Datetime('now', '-5 minutes', 'localtime');''',(user,med)).fetchall()
            if len(takes) > 0:
                return True
            else:
                return False
        elif comm_type == 'provider_weekly':
            takes = c.execute('''SELECT * FROM daily_take_table WHERE user_name = ? AND timing > (SELECT DATETIME('now', '-7 day')) ORDER BY timing ASC;''',(user,)).fetchall()
            conn.commit()
            c.close()
            conn.close()
            return parse_provider_weekly_status(user, takes)
        elif comm_type == 'send_provider_weekly':
            takes = c.execute('''SELECT * FROM daily_take_table WHERE user_name = ? AND timing > (SELECT DATETIME('now', '-7 day')) ORDER BY timing ASC;''',(user,)).fetchall()
            conn.commit()
            c.close()
            conn.close()
            email_body = parse_provider_weekly_status(user, takes)
            send_email(user, email_body)
            return 'Provider weekly email sent'

def parse_provider_weekly_status(user, takes):
    cnt = len(takes)
    if cnt == 0:
        return 'There is no medication taken within the past 7 days.'
    res = 'This is the daily medicine take status of ' + user + ':<br/>'
    
    daily = {}
    dates = []
    for i in range(cnt):
        med = takes[i][1]
        status = takes[i][2]
        datee = takes[i][3].split(' ')[0]
        timee = takes[i][3].split(' ')[1]
        if datee in daily:
            daily[datee].append((med, status, timee))
        else:
            daily[datee] = [(med, status, timee)]
            dates.append(datee)
    
    for datee in dates:
        res = res + "In date " + datee + ":<br/>"
        for takes in daily[datee]:
            res = res + takes[0] + " is taken " + takes[1] + " at " + takes[2] + "<br/>"
        res = res + "<br/>"
    
    return res

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
    msg['Subject']="[DayMax] " + user + " Medicine Taking Weekly Update"

    msg.attach(MIMEText(body, 'html'))
    s.send_message(msg)
    del msg
    s.quit()