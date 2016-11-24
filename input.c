#include "input.h"

#include <string.h>


static char input_buffer[INPUT_BUFFER_LENGTH];
static int input_length;


int input_check_complete()
{
	return input_buffer[input_length - 1] == '#';
}

void input_clear()
{
	memset(input_buffer, '\0', input_length);
	input_length = 0;
}

void input_append(char new_char)
{
	if(input_length < INPUT_BUFFER_LENGTH - 1) // we have space for our next character and string delimiter character
	{
		input_buffer[input_length] = new_char;
		input_length += 1;
	}
}

const char* input_get()
{
	return input_buffer;
}

char input_get_last()
{
	if(input_length > 0)
	{
		return input_buffer[input_length - 1];
	}
	return '\0';

}
