#include "config.h"

static int config_int[CONFIG_N_INT_ENTRIES] = {0};

static char config_string[CONFIG_N_STRING_ENTRIES][CONFIG_STRING_LENGTH] = {{0}};


int config_get_int(enum ConfigIntEntries entry)
{
	if(entry >= CONFIG_N_INT_ENTRIES)
	{
		// TODO: do something about this, don't just ignore it and move on
		return 0;
	}

	return config_int[entry];
}

const char* config_get_string(enum ConfigStringEntries entry)
{
	if(entry >= CONFIG_N_STRING_ENTRIES)
	{
		// TODO: do something about this, con't just ignore it and move on
		return 0;
	}

	return config_string[entry];
}
