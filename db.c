#define _GNU_SOURCE
// #define _POSIX_C_SOURCE 200809L

#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>


//implementation of getline function, this is not available in standard libaray of gcc compiler
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (lineptr == NULL || n == NULL || stream == NULL) {
        errno = EINVAL;
        return -1;
    }

    char *buf = *lineptr;
    size_t size = *n;
    ssize_t characters_read = 0;

    // If the buffer is NULL, initialize it with a size of 120
    if (buf == NULL) {
        size = 120;
        buf = (char *)malloc(size);
        if (buf == NULL) {
            errno = ENOMEM;
            return -1;
        }
    }

    int c;
    while ((c = fgetc(stream)) != EOF) {
        if (characters_read + 1 >= size) {
            size_t new_size = size + 120;
            char *new_buf = (char *)realloc(buf, new_size);
            if (new_buf == NULL) {
                errno = ENOMEM;
                return -1;
            }
            buf = new_buf;
            size = new_size;
        }

        buf[characters_read++] = (char)c;

        if (c == '\n') {
            break;
        }
    }

    // If end of file is reached and no characters were read, return -1
    if (characters_read == 0 && c == EOF) {
        return -1;
    }

    buf[characters_read] = '\0';

    *lineptr = buf;
    *n = size;

    return characters_read;
}


typedef enum{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
}MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS, 
    PREPARE_UNRECOGNIZED_STATEMENT
}PrepareResult;

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
}StatementType;

typedef struct {
    StatementType type;
} Statement;

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

InputBuffer* new_input_buffer(){
    InputBuffer * input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}

void print_prompt(){
    printf("db> ");
}

void read_input(InputBuffer* input_buffer){
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if(bytes_read <=0){
        printf("Errror reading input\n");
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

MetaCommandResult do_meta_command(InputBuffer* input_buffer){
    if(strcmp(input_buffer->buffer, ".exit") == 0){
        printf("Bye Bye...");
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    }
    else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement ){
    if(strncmp(input_buffer->buffer, "insert", 6) == 0){
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if(strcmp(input_buffer->buffer, "select") == 0){
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement){
    switch(statement->type){
        case(STATEMENT_INSERT):
            printf("This is where we would do an insert.\n");
            break;
        case(STATEMENT_SELECT):
            printf("This is where we would do a select.\n");
            break;
    }
}


void main(){
    InputBuffer* input_buffer = new_input_buffer();
    while(true){
        print_prompt();
        read_input(input_buffer);

        if(input_buffer->buffer[0] == '.'){
            switch(do_meta_command(input_buffer)){
                case(META_COMMAND_SUCCESS):
                    continue;
                case(META_COMMAND_UNRECOGNIZED_COMMAND):
                    printf("Unrecognized command '%s'\n",input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch(prepare_statement(input_buffer, &statement)){
            case(PREPARE_SUCCESS):
                break;
            case(PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
                continue;
        }
        execute_statement(&statement);
        printf("Executed.\n");
    }
}
