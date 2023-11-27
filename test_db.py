# In Python, the subprocess module provides functions for creating and interacting with pipes.
import subprocess 

def run_script(commands): 
	process = subprocess.Popen(
		"./db",
		stdin=subprocess.PIPE, # we need to be able to send input(write) to the program 
		stdout=subprocess.PIPE, # and get output(read) from it
		stderr=subprocess.PIPE, # as well as errors, 
		text=True
	)
	try:
		# input_bytes = "\n".join(commands).encode('utf-8')
		# Concatenate strings in commands with "\n", we need to hit enter after typing commands and \n = enter
		#output, error = process.communicate(input_bytes)
		output, error = process.communicate(("\n").join(commands) + "\n")	
	except UnicodeDecodeError as e:
		print(f"Unicode decode error: {e}")
		process = subprocess.Popen(
				"./db",
				stdin=subprocess.PIPE,
				stdout=subprocess.PIPE,
				stderr=subprocess.PIPE,
				text=True
		)
		output, error = process.communicate(("\n").join(commands) + "\n",  errors='replace')	
	#output = output.decode('utf-8', errors='replace')
	# process_communicate sends the concatenated string to the stdin of a subprocess(./db in our case)
	return output


def test_insert_and_retrieve():
	commands = [
				"insert 1 user1 person1@example.com",
				"select",
				".exit",
		]
	result = run_script(commands)
	# result is an array of strings, and assert checks if the following condition is met(True) or AssertError(False)
	assert "db > Executed." in result
	assert "db > (1, user1, person1@example.com)" in result
	assert "db > Executed." in result 
	assert "db > " in result

'''
	TODO:
		1. I have no idea why python is reading ".exi" instead of ".exit", so I have to hard code "\n"
'''

def test_table_full_error_message(): 
	script = [f"insert {i} user{i} person{i}@example.com" for i in range (1,1402)]
	script.append(".exit")

	result = run_script(script) 
	result_lines = result.split("\n")
	
	assert "db > Error: Table Full." in result_lines[-2]

def insert_string_max_length():
	long_username = "b" * 33
	long_email = "a" * 256
	
	script = [f"insert 1 {long_username} {long_email}", "select", ".exit"]
	result = run_script(script)
	print(result)
	assert "Executed." in result
	assert f"db > (1, {long_username}, {long_email})"
	assert "db > "

# this runs if the script is being run as main program (and not being imported)
if __name__ == "__main__":
	test_insert_and_retrieve()
	test_table_full_error_message() 
	insert_string_max_length()	
