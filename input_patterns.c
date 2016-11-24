#include "input_patterns.h"

#include <string.h>

#include "input.h"

#include "config.h"

static int security_code_attempts_remaining;

void start_security_code_attempts()
{
	input_clear();
	security_code_attempts_remaining = config_get_int(CONFIG_MAX_SECURITY_CODE_ATTEMPTS);
}

InputPatternSecurityCodeAttemptStatus check_security_code()
{
	// bail out early if we don't even have an attempt yet
	if(!input_check_complete()) {return ATTEMPT_NOT_COMPLETE;}

	// check that attempt is correct, then immediately clear to allow the next one
	int is_security_code_correct = !strncmp(input_get(), config_get_string(CONFIG_SECURITY_CODE), INPUT_BUFFER_LENGTH);
	input_clear();

	// keep correct count of attempts
	if(security_code_attempts_remaining)
	{
		security_code_attempts_remaining -= 1;
	}

	if(is_security_code_correct)
	{
		security_code_attempts_remaining = 0;
		return ATTEMPT_SUCCESS;
	}

	if(!security_code_attempts_remaining) {return ATTEMPT_FAILURE;}

	return ATTEMPT_ALLOW_AGAIN;
}
