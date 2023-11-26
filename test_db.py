# In Python, the subprocess module provides functions for creating and interacting with pipes.
import subprocess 

def run_script(commands): 
	process = subprocess.Popen(
		"./db",
		stdin=subprocess.PIPE, # we need to be able to send input(write) to the program 
		stdout=subprocess.PIPE, # and get output(read) from it
		stderr=subprocess.PIPE, # as well as errors
	)
	try:
		input_bytes = "\n".join(commands).encode('utf-8')
		# Concatenate strings in commands with "\n", we need to hit enter after typing commands and \n = enter
		output, error = process.communicate(input_bytes)
	except UnicodeDecodeError as e:
		print(f"Unicode decode error: {e}")
		process = subprocess.Popen(
				"./db",
				stdin=subprocess.PIPE,
				stdout=subprocess.PIPE,
				stderr=subprocess.PIPE,
		)
		output, error = process.communicate(input_bytes,  errors='replace')	
	output = output.decode('utf-8', errors='replace')
	# process_communicate sends the concatenated string to the stdin of a subprocess(./db in our case)
	return output


def test_insert_and_retrieve():
	commands = [
				"insert 1 user1 person1@example.com",
				"select",
				".exit",
		]
	result = run_script(commands)
	print(result)
	# result is an array of strings, and assert checks if the following condition is met(True) or AssertError(False)
	assert "db > Executed." in result
	assert "db > (1, user1, person1@example.com)" in result


'''

	TODO: 
		1. What is __name__ == "main"? (python or C?)
		2. Is there a main function in python3? 
		3. Install autocomplete for vim (plugin)
		4. Use pytest, it's more convinient
'''

# this runs if the script is being run as main program (and not being imported)
if __name__ == "__main__":
	test_insert_and_retrieve()
