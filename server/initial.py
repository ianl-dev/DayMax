import sys
sys.path.append('__HOME__/dayMax')
import update_info
import sqlite3
from datetime import date

db = '__HOME__/dayMax/user_info.db'

def request_handler(request,user_info_db=db):
	conn = sqlite3.connect(user_info_db)
	c = conn.cursor()  
	# c.execute('''DROP TABLE user_table;''')        # uncomment these lines if you want to clear the tables, run the page once and 
	# c.execute('''DROP TABLE medication_table;''')  # re-comment them
	# c.execute('''DROP TABLE alarms_table;''')
	# c.execute('''DROP TABLE daily_take_table;''')
	# c.execute('''DROP TABLE condition_table;''')
	c.execute('''CREATE TABLE IF NOT EXISTS user_table (user_name text, password text, birthdate date, gender text, email text, prov_fname text, prov_lname text, prov_email text);''')
	c.execute('''CREATE TABLE IF NOT EXISTS medication_table (user_name text, med_name text, dosage int, time_taken text, repetitions int, how_often_hours int, how_often_mins int, prereq_meal text);''')
	c.execute('''CREATE TABLE IF NOT EXISTS alarms_table (name text, med_name text, hour int, minute int, alarm_time text);''')
	user_result=""
	meds_result=""
	meds_exist = False
	if request['method']=='GET':
		name=""
		if ('name' in request['values']): name+=request['values']['name']
		if (len(name)!=0):
			info=c.execute('''SELECT * FROM user_table WHERE user_name = ? ;''',(name,)).fetchone()
			if (info==None):
				conn.commit() 
				conn.close()
				return '''User not found. Please indicate a valid user. Or visit the input page to add a new user: <a href="http://608dev-2.net/sandbox/sc/team103/dayMax/user_info.html" target="_blank">http://608dev-2.net/sandbox/sc/team103/dayMax/user_info.html</a>'''
			try:
				password_input=request['form']['pw']
				if(password_input!=info[1]):
					conn.commit() 
					conn.close()
					return "Incorrect password. Please refresh the page and insert the correct password to gain access to your records."
			except:
				conn.commit() 
				conn.close()
				return """
<html>
<head>
<style type="text/css">
	body {{
		align-items: center;
		background-color: #D8E0E6;
		display: flex;
		flex-flow: column;
		font-family: "Trebuchet MS", Helvetica, sans-serif;
		}}
	.top-menu {{
		background-color: #AD1023;
		float: right;
		margin: 0px;
		padding: 4px;
		position: relative;
		width: 100%;
		}}
	.daymax-logo {{
		color: white;
		font-family: "Trebuchet MS", Helvetica, sans-serif;
		margin: 4px;
		}}
	.top-line {{
		background-color: #524177;
		height: 12px;
		width: 100%;
		}}
	.invisible-text {{
		visibility: hidden;
		}}
	.password-form {{
		align-items: center;
		display: flex;
		flex-flow: column;
		}}
	.pw-input {{
		height: 50px;
		text-align: center;
		width: 500px;
		}}
	::placeholder {{
		font-family: "Trebuchet MS", Helvetica, sans-serif;
		font-size: 24px;
		text-align: center;
		}}
	.submit-button {{
		background-color: #524177;
		border-radius: 12px;
		border: none;
		color: white;
		cursor: pointer;
		font-family: "Trebuchet MS", Helvetica, sans-serif;
		font-size: 24px;
		padding: 8px 8px;
		width: 300px;
		}}
	#text {{display:none;color:#AD1023}}
</style>
<script>
	var input = document.getElementById("pw");
	document.addEventListener("keyup", detect); 
	function detect(event){{
		var text = document.getElementById("text");
		if (event.getModifierState("CapsLock")) {{
			text.style.display = "block";
		}} else {{
			text.style.display = "none"
		}}
	}};
</script>
</head>
<body>
	<div class="top-menu">
		<h1 class="daymax-logo"> DayMax </h1>
	</div>
	<div class="top-line">
		<div class="invisible-text"> test </div>
	</div>
	<form class="password-form" action="/sandbox/sc/team103/dayMax/initial.py?name={}" method="post">
		<h2> Please enter your password to access your medication data. </h2>
		<input type="password" id="pw" name="pw" value="" class="pw-input" placeholder="Enter password">
		<br><p>
		<input type="submit" class="submit-button">
		<p id="text">Caps Lock is on.</p>
	</form>
</body>
</html>
""".format(name)
			meds=c.execute('''SELECT * FROM medication_table WHERE user_name = ? ;''',(name,)).fetchall()
			if (len(meds)==0):
				meds_result="No medications listed.<br>"
				show_med_button=""
			else:
				meds_exist = True
				meds_result="""<tr class="table-header"> 
									<td>Medicine name</td>
									<td>Number of pills</td>
									<td>Start time</td>
									<td>Before/after meal</td>
									<td>Repetition</td>
							   </tr>"""
				show_med_button=""
		else:
			user_result="Please indicate a user when searching by adding ?name=[user name] to the end of url."
		conn.commit() 
		conn.close()
		if(len(user_result)!=0):
			return user_result
		else:
			if(meds_exist):
				for m in meds:
					meds_result+="""<tr>
										<td>{}</td> 
										<td> {} pills </td>
										<td> {} </td>
										<td> {} a meal </td>
								 """.format(m[1],m[2],m[3],m[7])
					if m[4]>1:
						meds_result+=" <td> {} more times a day, every {} hours and {} minutes. </td> </tr>".format(m[4]-1,m[5],m[6])
					else:
						meds_result+=" <td> No repetition </td></tr>"
				show_med_button=""" 
<div class="edit-delete-buttons">
	<form action="/sandbox/sc/team103/dayMax/update_midstep.py?name={}&update=med&update_type=change" method="post">
		<input class="edit-med-button" type="submit" value="Edit">
	</form>
	<form action="/sandbox/sc/team103/dayMax/update_midstep.py?name={}&update=med&update_type=delete" method="post">
		<input class="delete-med-button" type="submit" value="Delete">
	</form>
</div>
""".format(name,name)
			today=date.today()
			a=info[2].split('-')
			bday=date(int(a[0]),int(a[1]),int(a[2]))
			age=today.year - bday.year -((today.month, today.day) <(bday.month, bday.day))
			return """
<head>
	<style>
		body {{
			align-items: center;
			background-color: #9AA2B4;
			display: flex;
			flex-flow: column;
			font-family: "Trebuchet MS", Helvetica, sans-serif;
		}}
		.top-menu {{
			background-color: #AD1023;
			float: right;
			margin: 0px;
			padding: 4px;
			position: relative;
			width: 100%;
		}}
		.daymax-logo {{
			color: white;
			font-family: "Trebuchet MS", Helvetica, sans-serif;
			margin: 4px;
		}}
		.top-line {{
			background-color: #524177;
			height: 12px;
			width: 100%;
		}}
		.invisible-text {{
			visibility: hidden;
		}}
		.card {{
			align-items: center;
			background-color: #D8E0E6;
			border: solid #524177 8px;
			border-radius: 12px;
			border-width: 8px;
			display: flex;
			flex-flow: column;
			font-family: "Trebuchet MS", Helvetica, sans-serif;
			margin: 32px;
			width: 700px;
		}}
		.edit-button {{
			background-color: #EC724A;
			border-radius: 12px;
			border: none;
			color: white;
			cursor: pointer;
			font-family: "Trebuchet MS", Helvetica, sans-serif;
			font-size: 20px;
			font-weight: bold;
			height: 40px;
			padding: 10px 10px;
			width: 400px;
		}}
		table {{
			align-items: center;
			border: solid 2px;
			margin: 16px;
		}}
		tr, td {{
			border: solid 1px;
		}}
		.table-header {{
			background-color: #524177;
			color: white;
			font-weight: bold;
			text-align: center;
		}}
		.edit-delete-buttons {{
			display: flex;
			flex-flow: row;
			justify-content: space-between;
			width: 100%;
		}}
		.edit-med-button {{
			background-color: #524177;
			border-radius: 12px;
			border: none;
			color: white;
			cursor: pointer;
			font-family: "Trebuchet MS", Helvetica, sans-serif;
			font-size: 20px;
			font-weight: bold;
			height: 40px;
			margin-left: 12px;
			padding: 10px 10px;
			width: 200px;
		}}
		.delete-med-button {{
			background-color: #EC724A;
			border-radius: 12px;
			border: none;
			color: white;
			cursor: pointer;
			font-family: "Trebuchet MS", Helvetica, sans-serif;
			font-size: 20px;
			font-weight: bold;
			height: 40px;
			margin-right: 12px;
			padding: 10px 10px;
			width: 200px;
		}}
	</style>
	<script>
		function validate(form) {{
			console.log('in validate')
			var repetitions = parseInt(form["repetitions"].value);
			var hours = parseInt(form["how_often_hours"].value);
			var minutes = parseInt(form["how_often_mins"].value);
			var total = 60*hours+minutes;
			if (repetitions==1 && total!=0) {{
				console.log('time indicated but no reps')
				alert("Please confirm repetitions, time interval indicated is not 0.");
				return false;
			}}
			if (repetitions!=1 && total==0) {{
				console.log('more repetitions but no time')
				alert("Please confirm time interval, multiple repetitions were indicated.");
				return false;
			}}
		}}
	</script>
</head>
<body>
	<div class="top-menu">
		<h1 class="daymax-logo"> DayMax </h1>
	</div>
	<div class="top-line">
		<div class="invisible-text"> test </div>
	</div>
	<div class="card">
		<h1> Personal Information </h1>
		Name: {} <br>
		Age: {} <br>
		Gender: {} <br>
		Email: {} <br>
		Provider: {} <br>
		Provider's Email: {} <br><p>
		<form action="/sandbox/sc/team103/dayMax/update_midstep.py?name={}&update=contact" method="post">
			<input class="edit-button" type="submit" value="Update info">
		</form>
	</div>
	<div class="card">
		<h1>Medication Information</h1>
		<table>
			{}
		</table>
		{}
	</div>
	<div class="card">
		<p> To see the user guide on using the ESP32 device, please click the button below.  </p>
		<form name="form" action="/sandbox/sc/team103/dayMax/user_guide.html" method="get">
			<input class="edit-button" type="submit" value="Go to user guide">
		</form>
	</div>
	<div class="card">
		<h1>Add New Medication</h1><br>
		<form name="form" action="/sandbox/sc/team103/dayMax/initial.py?name={}" method="post"  onsubmit="return validate(this)">
			<label for="med_name">Name of Medication:</label>
			<input type="text" id="med_name" name="med_name" value="" required>
			<br><p>
			<label for="dosage">Dosage (num. of pills):</label>
			<input type="number" id="dosage" name="dosage" min="1" max="10" required>
			<br><p>
			<label for="time_taken">First taken at what time in one day?</label>
			<input type="time" id="time_taken" name="time_taken" required>
			<br><p>
			<label for="repetitions">How many times in a day do you take this medication?</label>
			<input type="number" id="repetitions" name="repetitions" min="1" max="5" required>
			<br><p>
			<label for="how_often_hours">Taken every </label>
			<input type="number" id="how_often_hours" name="how_often_hours" min="0" max="12" value=0 required>
			<label for="how_often_mins"> hours and </label>
			<input type="number" id="how_often_mins" name="how_often_mins" min="0" max="59" value=0 required>
			minutes
			<br><p>
			<label for="meal">Taken </label>
			<select id="meal" name="prereq_meal">
				<option value="before">before</option>
				<option value="after">after</option>
				<option value="unrelated to">unrelated to</option>
			</select> a meal<p>
			<input class="edit-button" type="submit" value="Add new medicine">
		</form>
	</div>
</body>
""".format(name, age, info[3], info[4], info[5] + " " + info[6], info[7], name, meds_result,show_med_button,name)
	
	elif request['method']=='POST':
		if('pw' in request['form'] and 'name' in request['values']):
			password_input=request['form']['pw']
			info=c.execute('''SELECT * FROM user_table WHERE user_name = ? ;''',(request['values']['name'],)).fetchone()
			if(password_input!=info[1]):
				conn.commit() 
				conn.close()
				return "Incorrect password. Please refresh the page and insert the correct password to gain access to your records."
			else:
				request['method']='GET'
				del request['content-type']
				del request['is_json']
				del request['data']
				conn.commit() 
				conn.close()
				return request_handler(request)
		if('birthdate' in request['form']):
			data=request['form']
			a=data['birthdate'].split('-')
			bday=date(int(a[0]),int(a[1]),int(a[2]))
			if(data['gender']=='other'):
				gender=data['gender_other']
			else:
				gender=data['gender']
			c.execute('''INSERT into user_table VALUES (?,?,?,?,?,?,?,?);''', (data['fname']+" "+data['lname'],data['password2'],bday,gender,data['email'],data['prov_fname'],data['prov_lname'],data['prov_email']))
			conn.commit() 
			conn.close()	
			return """
<meta http-equiv="refresh" content="0; URL='http://608dev-2.net/sandbox/sc/team103/dayMax/initial.py?name={}'" />
""".format(data['fname']+" "+data['lname'])
		elif('update' in request['form']):
			conn.commit() 
			conn.close()
			return update_info.request_handler(request,user_info_db)
		else:
			data=request['form']
			med=data['med_name'].upper()[0]+data['med_name'].lower()[1:]
			if(c.execute('''SELECT * FROM medication_table WHERE med_name = ? AND user_name=?;''',(med,request['values']['name'])).fetchone()!=None):
				conn.commit() 
				conn.close()	
				return "That medication already exists. Please go back to the previous page to edit the medication, {}.".format(med)
			c.execute('''INSERT into medication_table VALUES (?,?,?,?,?,?,?,?);''', (request['values']['name'],med,int(data['dosage']),data['time_taken'],int(data['repetitions']), int(data['how_often_hours']), int(data['how_often_mins']),data['prereq_meal']))
			#a=edit_alarms(request['values']['name'],data['med_name'],data['time_taken'],int(data['repetitions']), int(data['how_often_hours']), int(data['how_often_mins']))
			h=int(data['time_taken'][:2])
			m=int(data['time_taken'][-2:])
			often_hrs=int(data['how_often_hours'])
			often_mins=int(data['how_often_mins'])
			reps=int(data['repetitions'])
			for i in range(reps):
				a=(h+i*often_hrs)%24+(m+i*often_mins)//60
				b=(m+i*often_mins)%60
				c.execute('''INSERT into alarms_table VALUES (?,?,?,?,?);''', (request['values']['name'],med,a,b,"{:02d}".format(a)+":"+"{:02d}".format(b)))
			user=c.execute('''SELECT * FROM user_table WHERE user_name=?;''',(request['values']['name'],)).fetchone()
			conn.commit() 
			conn.close()
			request['method']='GET'
			del request['content-type']
			del request['is_json']
			del request['data']
			request['form']={'pw':user[1]}
			return request_handler(request)
		
