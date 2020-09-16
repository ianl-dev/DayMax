import sqlite3
import datetime
import requests
import json

"""
Get current time of the week in ET

The output will be in the form of 'hh:mm' if it's valid 'invalid' otherwise.
"""

def process_request():
    try:
        #time_api_res_raw = requests.get("http://api.timezonedb.com/v2.1/get-time-zone?key=V1CZG2TCNLIB&format=json&by=zone&zone=America/New_York")
        #time_api_res = json.loads(time_api_res_raw.text)
        #hhmmss = time_api_res["formatted"].split(' ')[1]
        #res = hhmmss.split(':')[0] + ':' + hhmmss.split(':')[1]
        res = str(datetime.datetime.now().hour) + ':' + str(datetime.datetime.now().minute) + ':' + str(datetime.datetime.now().second)
    except:
        res = 'invalid'
    return res

def request_handler(request):
    if request['method'] == 'GET':
        try:
            user = request['values']['user_name']
        except:
            return "Include user_name"
        res = process_request()

        # do a check if it's the time to send weekly update to provider
        # set to Sunday
        if datetime.datetime.today().weekday() == 0 and res == "00:11":
            email_sent = requests.get("http://608dev-2.net/sandbox/sc/team103/dayMax/get_daily_take_info.py?user_name="+ user +"&comm_type=send_provider_weekly")
            return email_sent

        return res