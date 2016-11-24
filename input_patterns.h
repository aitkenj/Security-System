typedef enum InputPatternSecurityCodeAttemptStatus
{
	ATTEMPT_NOT_COMPLETE,
	ATTEMPT_SUCCESS,
	ATTEMPT_ALLOW_AGAIN,
	ATTEMPT_FAILURE
} InputPatternSecurityCodeAttemptStatus;

void start_security_code_attempts();
InputPatternSecurityCodeAttemptStatus check_security_code();
