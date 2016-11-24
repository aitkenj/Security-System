#define CONFIG_STRING_LENGTH 128

enum ConfigIntEntries
{
	CONFIG_ALARM_ARM_GRACE_PERIOD,
	CONFIG_ALARM_TRIGGER_GRACE_PERIOD,
	CONFIG_DOOR_LOCK_TIMEOUT,
	CONFIG_INPUT_TIMEOUT,

	CONFIG_MAX_SECURITY_CODE_ATTEMPTS,

	CONFIG_N_INT_ENTRIES
};

enum ConfigStringEntries
{
	CONFIG_SECURITY_CODE,

	CONFIG_N_STRING_ENTRIES
};

int config_get_int(enum ConfigIntEntries entry);

const char* config_get_string(enum ConfigStringEntries entry);
