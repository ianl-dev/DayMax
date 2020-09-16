import sqlite3

user_info_db = '__HOME__/test2/user_info.db'

def request_handler(request):
	if request['method']=='GET':
		name=""
		return_type=""
		if ('name' in request['values']): name+=request['values']['name']
		if ('type' in request['values']): return_type=request['values']['type']
		try:
			conn = sqlite3.connect(user_info_db)
			c = conn.cursor()
			if (return_type=='list_all'):
				allList=c.execute('''SELECT * FROM alarms_table WHERE name = ? ORDER BY hour ASC, minute ASC;''',(name,)).fetchall()
				conn.commit()
				conn.close()
				return [(a[1],a[4]) for a in allList]
			else:
				conn = sqlite3.connect(user_info_db)
				c = conn.cursor()
				meds=c.execute('''SELECT * FROM medication_table WHERE user_name = ? ;''',(name,)).fetchall()
				conn.commit() 
				conn.close()
				out=""
				for m in meds:
					out+=m[1]+','+str(m[2])+','+str(m[4])+','+m[3]
					time=m[3]
					for i in range(m[4]-1):
						hr=int(time[:2])
						mins=int(time[-2:])
						time="{:02d}".format((hr+m[5])%24+(mins+m[6])//60)+":"+"{:02d}".format((mins+m[6])%60)
						out+=","+time
					out+=(",F,-" if m[7]=="unrelated to" else (",T,"+m[7]+",-"))
				return out
		except: 
			return "No medication found"
