import sqlite3
from datetime import date

user_info_db = '__HOME__/dayMax/user_info.db'

def request_handler(request):
	if request['method']=='GET':
		name=""
		if ('name' in request['values']): name+=request['values']['name']
		conn = sqlite3.connect(user_info_db)
		c = conn.cursor()
		s=c.execute('''SELECT * FROM user_table WHERE user_name = ? ;''',(name,)).fetchone()
		conn.commit() 
		conn.close()
		out=s[0]+","+s[4]+","+s[6]+","+s[7]
		return out