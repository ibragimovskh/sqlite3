#include <stdio.h>
#include <stdlib.h> 
#include <string.h>


#define COLUMN_USERNAME_SIZE 32 
#define COLUMN_EMAIL_SIZE 255

/*

    TODO: 
	1. Why use pointer for Statement? 
	2. Why "&" in execute_statement(&statement)?
 	3. Define limits for table inputs (varchar25, 225, and int)
	4. Handle the input from the user, implement table structure(row, column)  

    sqlitef front-end: 
	SQL compiler (input:string, output:bytecode)	
	or just, we need to parse sql syntax and return data
*/

// we need two enums: PREPARE and META
// we need statement (I don't know why we need struct for it tho)

// enum just makes program more readable, since in C you don't have false and true
// instead, you have 0 and 1, so imagine bunch of statements with 0's and 1's in them
// it would get very confusing, that's why we define enums to make code humanly-readable
// enums always go from 0 to n (in our case only 0, 1)

typedef struct {
	char *buffer; // the actual text ig
	size_t buffer_length; // is there a specific type for string size in c?
	ssize_t input_length; // ssize_t -> signed size_t (can be negative) 
} InputBuffer;

typedef enum {
	META_COMMAND_SUCCESS,
	META_COMMAND_UNRECOGNIZED
} MetaCommandResult; 

typedef enum {
	PREPARE_SUCCESS, 
	PREPARE_FAILURE
} PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef struct {
	uint32_t id; 
	char username[COLUMN_USERNAME_SIZE];
	char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
	StatementType type; // either 0 or 1 (used later in switch statement)
	Row row_to_insert; // only used by insert statement
} Statement; 	


InputBuffer* new_input_buffer() {
	// (InputBuffer*) for code-readability and error detection
	InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	input_buffer->buffer = NULL; 
	input_buffer->buffer_length = 0;
	input_buffer->input_length = 0;

	return input_buffer;
}  

void read_input(InputBuffer* input_buffer) {
	ssize_t bytes_read = getline( &(input_buffer->buffer), &(input_buffer->buffer_length), stdin  );
	if(bytes_read <= 0) {
		printf("Error reading input\n");
		exit(EXIT_FAILURE);
	}
	
	input_buffer->input_length = bytes_read - 1; // pure input
	input_buffer->buffer[bytes_read-1] = 0; 

}

void close_input_buffer(InputBuffer* input_buffer) {
	free(input_buffer -> buffer); //remember, there is no garbage collector
	free(input_buffer); 
}

// MetaCommandResult is just int (either 0 or 1) and that's what this function returns
MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
	if(strcmp(input_buffer->buffer, ".exit") == 0) {
		close_input_buffer(input_buffer);
		exit(EXIT_SUCCESS);
	} else {
		return META_COMMAND_UNRECOGNIZED;
	};
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
	// strncmp is used because after "insert" comes what actually needs to be inserted
	// that's why we only check first 6 characters of the line
	if(strncmp(input_buffer->buffer, "insert", 6) == 0) {
		statement -> type = STATEMENT_INSERT;
		return PREPARE_SUCCESS;
	}
	// but why don't we do the same here?
	if(strcmp(input_buffer->buffer, "select") == 0) {
		statement -> type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}
	return PREPARE_FAILURE;
}; 

void execute_statement(Statement* statement) {
	// we could use if statement, but there are too many conditions
	// so, use switch instead
	switch(statement -> type) {
		case(STATEMENT_INSERT):
			printf("This could be an insert statement.\n");
			break;
		case(STATEMENT_SELECT):
			printf("This could be a select statement.\n");
			break;
	}	
};

void print_prompt() {printf("db > ");}


int main( int argc, char* argv[] ) {
	
	InputBuffer* input_buffer = new_input_buffer(); 
	while(1) {
		print_prompt(); // sqlite3 one 
		read_input(input_buffer); 

		// Non-SQL statements like ".exit" are called meta-commands
		// In ASCII, "." == 46
		if(input_buffer->buffer[0] == 46) {
			switch(do_meta_command(input_buffer)) {
				case(META_COMMAND_SUCCESS):
					printf("%s\n", input_buffer->buffer);
					break;
				case(META_COMMAND_UNRECOGNIZED):
					printf("Unrecognized command %s\n", input_buffer->buffer);
					continue;
			}
		}			
	
		Statement statement; //wtf?
		// what are we preparing it for?
		switch(prepare_statement(input_buffer, &statement)) {
			case (PREPARE_SUCCESS): 
				break; 
			case (PREPARE_FAILURE): 
				printf("Unrecognized command %s\n", input_buffer->buffer);
				continue; // I guess prompt will just keep running
		}
		execute_statement(&statement);
		printf("Executed.\n");
	}	
	return 0; 
}
