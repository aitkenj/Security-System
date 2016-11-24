
#define INPUT_BUFFER_LENGTH 128

int input_check_complete();
void input_clear();
void input_append(char new_char);
const char* input_get();
