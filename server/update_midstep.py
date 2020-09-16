import sqlite3

user_info_db = '__HOME__/dayMax/user_info.db'

def request_handler(request):
	conn = sqlite3.connect(user_info_db)
	c = conn.cursor()  
	if(request['method']=='POST'):
		name=request['values']['name']
		if(request['values']['update']=='contact'):
			info=c.execute('''SELECT * FROM user_table WHERE user_name = ? ;''',(name,)).fetchone()
			out= """
<html>
<head>
<style type="text/css">
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
	.card-title {{
		font-family: "Trebuchet MS", Helvetica, sans-serif;
	}}
	.update-info-form {{
		align-items: center;
		display: flex;
		flex-flow: column;
	}}
	label {{
		font-size: 20px;
		font-weight: bold;
		margin: 8px;
	}}
	input {{
		height: 30px;
		margin: 8px;
		text-align: center;
		width: 300px;
	}}
	.submit-button {{
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
		width: 300px;
	}}
</style>
<script>
async function validateEmail(type) {{
  const access_key = '17cf32009d7d62b74113a800dfe7a934';
  if(type==''){{
    var email = document.forms['form']['email'].value;
  }}else{{
    var email = document.forms['form']['prov_email'].value;
  }}
  var bool = await getEmailAsync(email)
  .then(data => {{
    if(data==null){{
      console.log('not working');
      return false;
    }}
    else{{
      console.log(data)
      if (data.success==null){{
        if (data.format_valid==false){{
          alert("Please check the format of your "+type+"email address.");
          return false;
        }}else if (data.smtp_check==false){{
          if(data.did_you_mean!=""){{
            alert("Did you mean to insert this "+type+"email: " + data.did_you_mean);
            return false;
          }}else{{
            alert("Please enter a valid "+type+"email address.");
            return false;
          }}
        }}else{{
          return true;
          console.log('after true statment');
        }}
      }}else if (!data.success){{
        alert("Unable to verify "+type+"email address. Please check console for error message");
        console.log(data.error)
        return false;
      }}
    }}
  }});
  return bool;
}}

async function getEmailAsync(email) {{
  try{{
    let response = await fetch(`http://apilayer.net/api/check?access_key=534186fe475150269c4daed720a08f48&email=${{email}}`);
    return await response.json();
  }}catch(err){{
    console.error(err);
  }}
}} 

async function validate(type1,type2,form){{
  var result_email = await validateEmail(type1);
  var result_prov_email = await validateEmail(type2);
  console.log("email: " + result_email);
  console.log("prov_email: " + result_prov_email);
  if (result_email && result_prov_email){{
    console.log('here, submitting...');
    form.submit();
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
		<h1 class="card-title"> Update Personal Info </h1>
		<form name="form" class="update-info-form" action="/sandbox/sc/team103/dayMax/initial.py?name={}" method="post" onsubmit="validate('','provider ',this); return false;">
			<label for="email">Email:</label>
			<input type="email" id="email" name="email" value="{}" required>
			<label for="prov_fname">Provider First Name:</label>
			<input type="text" id="prov_fname" name="prov_fname" value="{}" required>
			<label for="prov_lname">Provider Last Name:</label>
			<input type="text" id="prov_lname" name="prov_lname" value="{}" required>
			<label for="prov_email">Provider Email:</label>
			<input type="email" id="prov_email" name="prov_email" value="{}" required>
			<input type="hidden" id="update" name="update" value="contact">
			<input class="submit-button" type="submit" value="Update Info">
		</form>
	</div>
</body>
</html>
""".format(name,info[4],info[5],info[6],info[7])
		elif(request['values']['update']=='med'):
			meds=c.execute('''SELECT * FROM medication_table WHERE user_name = ? ;''',(name,)).fetchall()
			options=""
			meds_result="""<tr class="table-header"> 
								<td>Medicine name</td>
								<td>Number of pills</td>
								<td>Start time</td>
								<td>Before/after meal</td>
								<td>Repetition</td>
							</tr>"""
			for m in meds:
				options+="""<option value="{}">{}</option>""".format(m[1],m[1])
				meds_result+="""<tr>
									<td>{}</td> 
									<td> {} pills </td>
									<td> {} </td>
									<td> {} a meal </td>
							 """.format(m[1],m[2],m[3],m[7])
				if m[4]>1:
					meds_result+="<td> {} more times a day, every {} hours and {} minutes. </td> </tr>".format(m[4]-1,m[5],m[6])
				else:
					meds_result+="<td> No repetition </td></tr>"
			if(request['values']['update_type']=='delete'):
				out= """
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
		.card-group {{
			display: flex;
			flex-flow: row;
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
		.delete-card {{
			align-items: center;
			background-color: #D8E0E6;
			border: solid #524177 8px;
			border-radius: 12px;
			border-width: 8px;
			display: flex;
			flex-flow: column;
			margin: 32px;
			width: 250px;
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
		label {{
			font-size: 20px;
			margin: 8px;
			text-align: center;
		}}
		.update-info-form {{
			align-items: center;
			display: flex;
			flex-flow: column;
		}}
		.submit-button {{
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
			width: 150px;
		}}
	</style>
</head>
<body>
	<div class="top-menu">
		<h1 class="daymax-logo"> DayMax </h1>
	</div>
	<div class="top-line">
		<div class="invisible-text"> test </div>
	</div>
	<div class="card-group">
		<div class="card">
			<h2>Current Medication</h2>
			<table>
				{}
			</table>
		</div>
		<div class="delete-card">
			<h2> Delete Medication </h2>
			<form class="update-info-form" action="/sandbox/sc/team103/dayMax/initial.py?name={}" method="post">
				<label for="med_name">Choose a medication to delete: </label>
				<select id="med_name" name="med_name">
					{}
				</select><br><p>
				<input type="hidden" id="update" name="update" value="med">
				<input type="hidden" id="update_type" name="update_type" value="delete">
				<input class="submit-button" type="submit" value="Delete">
			</form>
		</div>
	</div>
	<h4> Or go back to the previous page to cancel. </h4>
</body>
""".format(meds_result,name,options)
			elif(request['values']['update_type']=='change'):
				out= """
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
		.card-group {{
			display: flex;
			flex-flow: row;
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
		.edit-card {{
			align-items: center;
			background-color: #D8E0E6;
			border: solid #524177 8px;
			border-radius: 12px;
			border-width: 8px;
			display: flex;
			flex-flow: column;
			margin: 32px;
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
		.submit-button {{
			background-color: #EC724A;
			border-radius: 12px;
			border: none;
			color: white;
			cursor: pointer;
			display: block;
			font-family: "Trebuchet MS", Helvetica, sans-serif;
			font-size: 20px;
			font-weight: bold;
			height: 40px;
			margin: 0 auto;
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
	<div class="card-group">
		<div class="card">
			<h2>Current Medication</h2>
			<table>
				{}
			</table>
		</div>
		<div class="edit-card">
			<h2> Edit Medication </h2>
			<form name="form" action="/sandbox/sc/team103/dayMax/initial.py?name={}" method="post" onsubmit="return validate(this)">
				<label for="med_name">Choose a medication to edit: </label>
				<select id="med_name" name="med_name" >
					{}
				</select><br><p>
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
				<input type="hidden" id="update" name="update" value="med">
				<input type="hidden" id="update_type" name="update_type" value="change">
				<input class="submit-button" type="submit" value="Edit medicine">
			</form>
		</div>
	</div>
	<h3>Or go back to the previous page to cancel.</h3>
</body>
""".format(meds_result,name,options)
		
		conn.commit()
		conn.close()
		return out

