import sqlite3
import datetime

user_info_db = '__HOME__/dayMax/user_info.db'

def request_handler(request):
	conn = sqlite3.connect(user_info_db)
	c = conn.cursor()  
	if(request['method']=='GET'):
		try:
			user=request['values']['name']
			action=request['values']['action']
			time=request['values']['time']
			hr=time.split(":")[0]
			mins=time.split(":")[1]
		except:
			return "Parameters missing. Make sure to include both 'name', 'action' and 'time' tags."
		if(action=='next'):
			upcoming=c.execute('''SELECT * FROM alarms_table WHERE name = ? AND (hour > ? OR (hour = ? AND minute >= ?)) ORDER BY hour ASC, minute ASC;''',(user,hr,hr,mins)).fetchall()
			out= ""
			if len(upcoming)!=0:
				for u in upcoming:
					time=datetime.datetime.now()
					time=time-datetime.timedelta(hours=time.hour, minutes=time.minute, seconds=time.second, microseconds=time.microsecond)
					taken_today=c.execute('''SELECT * FROM daily_take_table WHERE user_name = ? AND med_name = ? AND timing >= ? AND alarm_time = ? ORDER BY alarm_time ASC;''',(user,u[1],time,u[4])).fetchall()
					med_info=c.execute('''SELECT * FROM medication_table WHERE user_name = ? AND med_name = ?;''',(user,u[1])).fetchone()
					if len(taken_today)==0:
						out=str(med_info[1])+","+str(med_info[2])+",1,"+u[4]+(",F,-" if med_info[7]=="unrelated to" else (",T,"+med_info[7]+",-"))
						break

		elif(action=='status'):
			try:
				status_type=request['values']['type']
			except:
				return "Parameter missing. Make sure to include the 'type' tag."
			time=datetime.datetime.now()
			time=time-datetime.timedelta(hours=time.hour, minutes=time.minute, seconds=time.second, microseconds=time.microsecond)
			taken_today=c.execute('''SELECT * FROM daily_take_table WHERE user_name = ?  AND timing >= ?;''',(user,time)).fetchall()
			taken_list=[]
			for t in taken_today:
				taken_list.append(str(t[4])+" " +str(t[1]))
			
			upcoming=c.execute('''SELECT * FROM alarms_table WHERE name = ? AND (hour > ? OR (hour = ? AND minute >= ?)) ORDER BY hour ASC, minute ASC;''',(user,hr,hr,mins)).fetchall()
			upcoming_list=[]
			for u in upcoming:
				upcoming_list.append(str(u[4])+" "+str(u[1]))
			
			temp_hr = hr - 1;
			temp_mins = mins + 30;
			if temp_mins >= 60:
				temp_hr += 1;
				temp_mins -= 60;
			pending=c.execute('''SELECT * FROM alarms_table WHERE name = ? AND (hour > ? OR (hour = ? AND minute >= ?)) AND (hour < ? OR (hour = ? AND minute <= ?)) ORDER BY hour ASC, minute ASC;''',(user,temp_hr,temp_hr,temp_mins, hr, hr, mins)).fetchall()
			pending_list=[]
			for u in pending:
				pending_list.append(str(u[4])+" "+str(u[1]))
			
			alarms_before=c.execute('''SELECT * FROM alarms_table WHERE name = ? AND (hour < ? OR (hour = ? AND minute < ?)) ORDER BY hour ASC, minute ASC;''',(user,hr,hr,mins)).fetchall()
			all_list=[]
			for a in alarms_before:
				all_list.append(str(a[4])+" "+str(a[1]))

			out=""
			if status_type=='missed':
				for a in all_list:
					if a not in taken_list:
						out+=a+"-"
			elif status_type=='upcoming':
				for u in upcoming_list:
					if u not in taken_list:
						out+=u+"-"
			elif status_type=='pending':
				for p in pending_list:
					if p not in taken_list:
						out+=p+"-"
			elif status_type=='taken':
				for t in taken_list:
					out+=t+"-"

		conn.commit()
		conn.close()
		return out if out != "" else "-"
