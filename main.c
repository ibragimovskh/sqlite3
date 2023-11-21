#include <stdio.h>

typedef struct InputBuffer{
	char *buffer; // the actual text ig
	size_t buffer_length; // is there a specific type for string size in c?
	ssize_t input_length; // ssize_t -> signed size_t (can be negative) 
};

void print_prompt() {printf("db > ");}

void read_input(InputBuffer* input_buffer) {
	ssize_t bytes_read = getline( &(input_buffer->buffer), &(input_buffer->buffer_length), stdin  );
	if(bytes_read <= 0) {
		printf("Error reading input\n");
		exit(EXIT_FAILURE);
	}
	
	input_buffer->input_length = bytes_read - 1; // pure input
	input_buffer->[bytes_read-1] = 0; 

}

void close_input_buffer(InputBuffer* input_buffer) {
	free(input_buffer -> buffer); //remember, there is no garbage collector
	free(input_buffer); 
}

int main( int argc, char* argv[] ) {
	
	InputBuffer* input_buffer = new_input_buffer(); 
	while(true) {
		print_prompt(); // sqlite3 one 
		read_input(input_buffer); 
		
		if(strcmp(input_buffer->buffer , ".exit")==0){
			close_input_buffer(input_buffer);
			exit(EXIT_SUCCESS); 
		}else {
			printf("Unrecognized command %s \n", input_buffer->buffer); 
		}
	}	

	return 0; 
}
