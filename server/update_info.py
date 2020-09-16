import sys

sys.path.append('__HOME__/dayMax')
import initial

import sqlite3
import datetime
import requests

# user_info_db = '__HOME__/dayMax/user_info.db'
db = '__HOME__/dayMax/user_info.db'

"""

Purpose: whenever a user takes their meds, they notify the system by responding to the buzzer, which sends a POST request. 
         This script details what to do with the updated information.
         
Goal: To process POST requests and store data appropriately AFTER user initialization.

Type of Data collected:
    Medication Name (time)
    Time take (HH:MM)
    Pre-requisite
    Dosage (Int)

Type of update command handled:
1. Clear all information: user, medication info, and daily medication info
   Method: POST with body user_name = 'clear all'
2. User updating their personal contact (user email, provider first/last names, provider email)
   Method: POST with body update = 'contact', 
           and include ALL the information in the body variable email, prov_fname, prov_lname, prov_email
3. User deleting 1 medication
   Method: POST with body update = 'med', update_type = 'delete'
           and include med_name information in the body variable 
4. User updating 1 medication
   Method: POST with body update = 'med', update_type = 'change'
           and include ALL the information in the body variable med_name, dosage, time_taken, prereq_meal
5. User input their medication taking status
   Method: POST with body update = 'daily_take'
           and include ALL the information in the body variable med_name, status
           status can be either one of "timely", "early", "late"
6. User wants to update their medical conditions at-the-moment
   Method: POST with body update = 'cond'
           and include ALL the information in the body variable 
           user_name, heart_rate, respiration_rate, systolic_pressure, diastolic_pressure, blood_sugar
"""

def request_handler(request,user_info_db=db):
    conn = sqlite3.connect(user_info_db)
    c = conn.cursor()
    
  # Print existing samples from USER/MEDICATION table
#    sample = []
#    c.execute("SELECT * FROM condition_table")
#    [sample.append(row) for row in c.fetchall()]
#    return sample
    
    if request['method']=='POST':
        try:
            user = request['form']['user_name']
        except:
            try:
                user = request['values']['name']
            except:
                return 'Name not found'
        # Create a database table for daily medicine taking status
        c.execute('''CREATE TABLE IF NOT EXISTS daily_take_table (user_name text, med_name text, status text, timing timestamp, alarm_time text);''')
        # Create a database for user's condition
        c.execute('''CREATE TABLE IF NOT EXISTS condition_table (user_name text, heart_rate int, respiration_rate int, sys_pressure int, dia_pressure int, blood_sugar int, timing timestamp);''')
        # CASE 1: Clear existing user info
        if user == 'clear all': # for the MVP, the system resets all user(s) data at once

            c.execute('''DROP TABLE user_table;''') 
            c.execute('''DROP TABLE medication_table;''')
            c.execute('''DROP TABLE daily_take_table;''')
            c.execute('''DROP TABLE condition_table;''')
            c.execute('''DROP TABLE alarms_table;''')

            conn.commit()
            c.close()
            conn.close()
            return 'Cleared all user data.' # Next time, system performs a http_request using initial.py script to add new values to tables
        name = c.execute("SELECT * FROM user_table WHERE user_name = ?", (user,)).fetchall()
        # User is valid (initialized before)
        if name:
            # CASE 2:  User wants to update their contact info (email, provider f/l names, provider email)
            if request['form']['update'] == "contact":  # for the MVP, user updates all contact info all at once
                try:
                    email = request['form']['email']
                    prov_f = request['form']['prov_fname']
                    prov_l = request['form']['prov_lname']
                    prov_email = request['form']['prov_email']
                except:
                    conn.commit()
                    conn.close()
                    return "Missing needed parameter(s) to update contact info."
                c.execute('UPDATE user_table SET email=?, prov_fname=?, prov_lname=?, prov_email=? WHERE user_name=?', (email,prov_f,prov_l,prov_email, user))
                conn.commit()
                conn.close()
                request['method']='GET'
                del request['content-type']
                del request['is_json']
                del request['data']
                request['form']={'pw':name[0][1]}
                return initial.request_handler(request,user_info_db)
            elif request['form']['update'] == 'med':
                try:
                    update_type = request['form']['update_type']
                except:
                    conn.commit()
                    c.close()
                    conn.close()
                    return "Missing needed update_type."
                # CASE 3: Delete a medication
                if update_type == 'delete':
                    try:
                        med = request['form']['med_name']
                    except:
                        conn.commit()
                        c.close()
                        conn.close()
                        return "Missing needed medication name."
                    c.execute('DELETE FROM medication_table WHERE user_name=? AND med_name=?;', (user, med))
                    c.execute('DELETE FROM alarms_table WHERE name=? AND med_name=?;', (user, med))
                    conn.commit()
                    conn.close()
                    request['method']='GET'
                    del request['content-type']
                    del request['is_json']
                    del request['data']
                    request['form']={'pw':name[0][1]}
                    return initial.request_handler(request,user_info_db)
                # CASE 4: Change a medication
                elif update_type == 'change':
                    try:
                        med = request['form']['med_name']
                        dosage = int(request['form']['dosage'])
                        time_taken = request['form']['time_taken']
                        prereq_meal = request['form']['prereq_meal']
                        repetitions = int(request['form']['repetitions'])
                        how_often_hours = int(request['form']['how_often_hours'])
                        how_often_mins = int(request['form']['how_often_mins'])
                    except:
                        conn.commit()
                        c.close()
                        conn.close()
                        return "Missing needed parameter(s) for updating a medication."
                    c.execute('UPDATE medication_table SET dosage = ? , time_taken = ? , prereq_meal = ? , repetitions = ? , how_often_hours = ? , how_often_mins = ? WHERE user_name=? AND med_name=?;',  (dosage, time_taken, prereq_meal, repetitions, how_often_hours, how_often_mins, user, med))
                    c.execute('DELETE FROM alarms_table WHERE name=? AND med_name=?;', (user, med))
                    h=int(time_taken[:2])
                    m=int(time_taken[-2:])
                    for i in range(repetitions):
                        a=(h+i*how_often_hours)%24+(m+i*how_often_mins)//60
                        b=(m+i*how_often_mins)%60
                        c.execute('''INSERT into alarms_table VALUES (?,?,?,?,?);''', (user,med,a,b,"{:02d}".format(a)+":"+"{:02d}".format(b)))
                    conn.commit()
                    conn.close()
                    request['method']='GET'
                    del request['content-type']
                    del request['is_json']
                    del request['data']
                    request['form']={'pw':name[0][1]}
                    return initial.request_handler(request,user_info_db)
            # CASE 5: User has just taken a medication (user, medication, status, timing)
            elif request['form']['update'] == 'daily_take':
                try:
                    med = request['form']['med_name']
                    med_status = request['form']['status']
                except:
                    conn.commit()
                    c.close()
                    conn.close()
                    return "Missing needed parameter(s)."
                time=datetime.datetime.now()
                d=time.hour*60+time.minute
                alarm_list={}
                alarms=c.execute('''SELECT * FROM alarms_table WHERE name = ? AND med_name = ?;''',(user,med)).fetchall()
                for a in alarms:
                    alarm_list[abs(a[2]*60+a[3]-d)]=(a[2],a[3])
                alarm_time=alarm_list[min(alarm_list)]
                c.execute('INSERT into daily_take_table VALUES (?,?,?,?,?);', (user, med, med_status, datetime.datetime.now(), "{:02d}".format(alarm_time[0])+":"+"{:02d}".format(alarm_time[1])))
                conn.commit()
                c.close()
                conn.close()
                return "Info updated. Your last medication taken is {} at {}".format(med, datetime.datetime.now())   


            # CASE 6: User wants to update their medical conditions at-the-moment
            elif request['form']['update'] == 'cond':
                try:
                	# casting to int for data manipulation
                    heart = int(request['form']['heart_rate'])
                    breath = int(request['form']['respiration'])
                    sys_pressure = int(request['form']['systolic'])
                    dia_pressure = int(request['form']['diastolic'])
                    sugar = int(request['form']['blood_sugar'])
                except:
                    conn.commit()
                    c.close()
                    conn.close()
                    return "Missing needed parameter(s)."
                # Record the details and time when a bio-measurement is taken
                c.execute('INSERT into condition_table VALUES (?,?,?,?,?,?,?);', (user, heart, breath, sys_pressure, dia_pressure, sugar, datetime.datetime.now()))
                conn.commit()
                c.close()
                conn.close()

                # Do condition checkings
                checkConditionGET(user, 3, 'heart_rate')
                checkConditionGET(user, 3, 'respiration_rate')
                checkConditionGET(user, 3, 'systolic_pressure')
                checkConditionGET(user, 3, 'diastolic_pressure')
                checkConditionGET(user, 3, 'blood_sugar')
                return "Info updated. Your latest measurement is taken at {}, with a resting heart rate of {} BPM, respiration rate of {} BPM, blood pressure at {}/{} mmHg and blood glucose level at {} mg/dL.".format(datetime.datetime.now(), heart, breath, sys_pressure, dia_pressure, sugar)
        else: # invalid
            return "User name not found. Fail to update"     
    else:
        return "Unable to update the server. Try Posting."


def checkConditionGET(user, num, condition):
    res = requests.get("http://608dev-2.net/sandbox/sc/team103/dayMax/condition_check.py?user_name="+ user +"&command=average_over_days&num_day="+ str(num) +"&condition="+ condition + "&email=True")
    return res.text

