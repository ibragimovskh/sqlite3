#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <stdint.h>

#define COLUMN_USERNAME_SIZE 32 
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100 

/*
    TODO: 
	1. serialize_row()
	 - Serialization - converting data strcture into format that can be stored or transmitted (bytecode)
	2. Why "&" in execute_statement(&statement)?
	 - Because, that's how you modify the instance, and not the copy of it (copy isn't saved outside of function)
 	3. Define limits for table inputs (varchar25, 225, and int)
	4. Handle the input from the user, implement table structure(row, column)  

    sqlitef front-end: 
	SQL compiler (input:string, output:bytecode)	
	or just, we need to parse sql syntax and return data
*/

// we need two enums: PREPARE and META
// we need statement 

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
	PREPARE_SYNTAX_ERROR,
	PREPARE_UNRECOGNIZED_COMMAND
} PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef struct {
	uint32_t id; 
	char username[COLUMN_USERNAME_SIZE];
	char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
	// why do we need num_rows? 
	uint32_t num_rows;
	void* pages[TABLE_MAX_PAGES];

} Table;

typedef struct {
	StatementType type; // either 0 or 1 (used later in switch statement)
	Row row_to_insert; // only used by insert statement
} Statement; 	

typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } ExecuteResult;

/*
	Very Important! Struct* is just saying that it is a pointer, it's not dereferencing anything 
	line below is a syntax trick we can use to figure out the size of an Attribute without initiating an instance of Struct
*/
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email); 
const uint32_t ID_OFFSET = 0; 
const uint32_t USERNAME_OFFSET = ID_SIZE + ID_OFFSET;
const uint32_t EMAIL_OFFSET = USERNAME_SIZE + USERNAME_OFFSET;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

// Table points to the pages of rows
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE; 
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

// Copying to Table
void serialize_row(Row *source, void *destination){
	memcpy( destination + ID_OFFSET, &(source->id), ID_SIZE ); 
	memcpy( destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
	memcpy( destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE );
}
// Copying from table 
void deserialize_row(void* source, Row* destination) {
	memcpy( &(destination->id), source + ID_OFFSET, ID_SIZE );
	memcpy( &(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE );
	memcpy( &(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

/*
 void function doesn't return anything, void* returns a pointer
 same with row_slot, it doesn't perform operations on table, it just returns a pointer
 to the page(address) where the operation needs to be performed
 important detail: even if the page was NULL, after the space is allocated,
 we still use the row_num and don't just put data in wherever is free
 e.g.: row_num = 2, page is NULL
 	1. allocate memory
	2. put data into SECOND row, first stays empty
*/
void* row_slot(Table* table, uint32_t row_num) {  
	uint32_t page_num = row_num / ROWS_PER_PAGE; 
	void* page = table -> pages[page_num]; 
	if(page == NULL) {
		// why not just page = malloc(PAGE_SIZE)?
		// because, it would make page point to the allocated memory(randomly)
		// and table->pages[page_num] would still be NULL 
		page = table->pages[page_num] = malloc(PAGE_SIZE); 
	} 
	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	// address still needs offset in bytes
	uint32_t byte_offset = row_offset * ROW_SIZE;

	return page + byte_offset;
} 

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
		char args_num = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id), &(statement->row_to_insert.username), &(statement->row_to_insert.email));
		if(args_num < 3) {
			return PREPARE_SYNTAX_ERROR;
		}
		return PREPARE_SUCCESS;
	}
	// but why don't we do the same here?
	if(strcmp(input_buffer->buffer, "select") == 0) {
		statement -> type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}
	return PREPARE_SYNTAX_ERROR;
}; 

ExecuteResult execute_insert(Statement* statement, Table* table ) {
	if(table->num_rows >= TABLE_MAX_ROWS) {
		return EXECUTE_TABLE_FULL; 
	}
	/*
		Row* is just a type (pointer to Row), not dereferencer
		Assigning the addr of rwtoinsrt to row_to_insert
		There is another way to do it, but that would copy the whole thing to row_to_insert
		And then get the address to the newly copied data (so that you have the same data stored twice)

	*/
	Row* row_to_insert = &(statement->row_to_insert);
	serialize_row(row_to_insert, row_slot(table, table->num_rows));
	table->num_rows += 1;

	return EXECUTE_SUCCESS;
} 

void print_row(Row* row) {
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);	
}

ExecuteResult execute_select(Statement* statement, Table* table) {
	Row row; 
	// this is a lot, but it is beautiful and makes absolute sense
	for(int i = 0; i < table->num_rows; i++) {
		deserialize_row(row_slot(table,i), &row);
		print_row(&row); 
	}
		
	return EXECUTE_SUCCESS;	
}

ExecuteResult execute_statement(Statement* statement, Table* table) {
	
	// need to return the function result, it is used later in main()
	switch(statement -> type) {
		case(STATEMENT_INSERT):
			return execute_insert(statement, table);
		case(STATEMENT_SELECT):
			return execute_select(statement, table); 
	}	
};

Table* new_table() {
	Table* table = (Table*)malloc(sizeof(Table));
	table->num_rows = 0; 

	// my assumption was wrong, page wasn't null by default
	for(int i = 0; i < TABLE_MAX_PAGES; i++) {
		table->pages[i]=NULL;

	}
	return table;
} 

// Write deconstructor for table
void free_table(Table* table) {
	for(int i = 0; i < table->num_rows; i++) {
		// free() only works on pointers that were returned my malloc()
		// in this case, pages[i], since page is a pointer from malloc returned by row_slot
		free(table->pages[i]);
	}
	free(table);
} 

void print_prompt() {printf("db > ");}


int main( int argc, char* argv[] ) {
	Table *table = new_table(); 
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
			case (PREPARE_SYNTAX_ERROR):
				printf("Syntax error. Could not parse statement.\n");
				continue;
			case (PREPARE_UNRECOGNIZED_COMMAND): 
				printf("Unrecognized command %s\n", input_buffer->buffer);
				continue; // I guess prompt will just keep running
		}
		// so statement->type is the same as (*statement).type, you have direct access
		switch(execute_statement(&statement, table)) {
			case (EXECUTE_SUCCESS):
				printf("Executed.\n");
				break;
			case (EXECUTE_TABLE_FULL):
				printf("Error: Table Full. \n");
				break;
		}
	}	
	return 0; 
}
