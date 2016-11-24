#include "driverlib.h"

#include "alarm_system_states.h"

#include "input_patterns.h"

#include "config.h"

typedef enum SystemState
{
	STATE_ERROR,
	STATE_POWER_ON,
	STATE_FIRST_RUN,
	STATE_DISARMED,
	STATE_ARMING,
	STATE_ARMED,
	STATE_TRIGGERING,
	STATE_TRIGGERED,
	STATE_CONFIGURATION,
	STATE_DOOR_LOCKING,
	STATE_DOOR_UNLOCKING,
	STATE_POWER_OFF
} SystemState;

// there is no reason these would be needed in any other file,
// so we'll make them all static and give them simple names
static SystemState run_error();
static SystemState run_power_on();
static SystemState run_first_run();
static SystemState run_disarmed();
static SystemState run_arming();
static SystemState run_armed();
static SystemState run_triggering();
static SystemState run_triggered();
static SystemState run_configuration();
static SystemState run_door_locking();
static SystemState run_door_unlocking();
static SystemState run_power_off();


// Define an array of function pointers covering all states.
// Ignore the syntactical weirdness here.
// The name of the array is run_state and it contains
// as many states as there are in the SystemState enum.
// These should be in the same order as the SystemState enum,
// or bad things happen.
SystemState (*run_state[STATE_POWER_OFF + 1])() =
{
		run_error,
		run_power_on,
		run_first_run,
		run_disarmed,
		run_arming,
		run_armed,
		run_triggering,
		run_triggered,
		run_configuration,
		run_door_locking,
		run_door_unlocking,
		run_power_off
};



static SystemState state_machine_current = STATE_ERROR;

// The current state must not be wrong, so must not be set
// by mistake, so for safety we use this function to
// actually access it everywhere outside state_machine_run().
// Needing to know the current state is usually a design
// flaw anyway, so this should be used rarely.
SystemState state_machine_get_current()
{
	return state_machine_current;
}


void state_machine_run()
{
	// prev and next states are only relevant to state_machine_run,
	// so are declared inside the function
	static SystemState prev_state = STATE_ERROR; //
	static SystemState next_state = STATE_POWER_ON; //

	while(1)
	{
		MAP_WDT_A_clearTimer();

		// shove states back to make room for next state
		prev_state = state_machine_current;
		state_machine_current = next_state;

		// reset any information that may be accidentally carried across states
		if(prev_state != state_machine_current)
		{
			input_clear();
			countdown_start(0, prev_state); // prevent countdown from running in next state
		}

		// run the current state to find the next state
		next_state = run_state[state_machine_current](prev_state == state_machine_current);

		if(next_state > STATE_POWER_OFF)
		{
			next_state = STATE_ERROR;
		}
	}
}



SystemState run_error(int was_last_state)
{
	log_error_state();

	while(1)
	{
		// run out the watchdog timer
	}

	return STATE_ERROR;
}

SystemState run_power_on(int was_last_state)
{
	// do any fancy initialization stuff,
	// then read the state we want to restore into from persistent memory,
	// and return that state

	return persistence_get_last_recorded_state();
}

SystemState run_first_run(int was_last_state)
{
	// do any first-run configuration

	return STATE_DISARMED;
}

SystemState run_disarmed(int was_last_state)
{
	static int in_input_mode = 0;

	if(!was_last_state)
	{
		// reset state information
		in_input_mode = 0;
	}

	// update the information on the display regularly,
	display_idle_screen();

	// enter input mode on start of input
	if(input_get_last() && !in_input_mode)
	{
		display_menu_disarmed_state();

		// start countdown to exit input mode
		countdown_start(config_get_int(CONFIG_INPUT_TIMEOUT), STATE_DISARMED);
		// clear input to accept an actual selection
		input_clear();

		in_input_mode = 1;
	}

	if(in_input_mode)
	{
		// get the selection, then clear because we don't need it again
		char selection = input_get_last();
		input_clear();


		// exit input mode on timeout
		if(!countdown_running(STATE_DISARMED))
		{
			in_input_mode = 0;
			clear_menu_disarmed_state();
		}

		// exit the disarmed state if we have a good selection
		if(selection)
		{
			return disarmed_state_menu_select(selection);
		}
	}

	return STATE_DISARMED;
}

SystemState run_arming(int was_last_state)
{
	if(!was_last_state)
	{
		countdown_start(config_get_int(CONFIG_ALARM_ARM_GRACE_PERIOD), STATE_ARMING);
		start_security_code_attempts();
	}

	if(countdown_running(STATE_ARMING))
	{
		// update the display to indicate that we're counting down,

		// allow security code to cancel
		switch(check_security_code())
		{
		case ATTEMPT_SUCCESS: return STATE_DISARMED;
		case ATTEMPT_FAILURE: return STATE_TRIGGERED;
		}

		return STATE_ARMING;
	}

	return STATE_ARMED;
}

SystemState run_armed()
{
	while(1)
	{
		// check for sensor trigger,
		// update the information on the display regularly,


		// allow security code to cancel
		switch(check_security_code())
		{
		case ATTEMPT_SUCCESS: return STATE_DISARMED;
		case ATTEMPT_FAILURE: return STATE_TRIGGERED;
		}
	}
}

SystemState run_triggering()
{
	countdown_start(config_get_int(CONFIG_ALARM_TRIGGER_GRACE_PERIOD));
	while(countdown_running())
	{
		// update the display to indicate that we're counting down,

		// allow security code to cancel
		switch(check_security_code())
		{
		case ATTEMPT_SUCCESS:
			countdown_stop();
			return STATE_DISARMED;
		case ATTEMPT_FAILURE:
			countdown_stop();
			return STATE_TRIGGERED;
		}
	}

	return STATE_TRIGGERED;
}

SystemState run_triggered()
{
	while(1)
	{
		// do alarm stuff

		switch(check_security_code())
		{
		case ATTEMPT_SUCCESS: return STATE_DISARMED;
		}
	}
}

SystemState run_configuration()
{
	// all kinds of configuration stuff

	return STATE_DISARMED;
}

SystemState run_door_locking()
{
	// lock the door

	door_lock();
	return STATE_DISARMED;
}

SystemState run_door_unlocking()
{
	countdown_start(config_get_int(CONFIG_DOOR_LOCK_TIMEOUT), STATE_DOOR_UNLOCKING);
	start_security_code_attempts();
	while(countdown_running())
	{
		// update the display to indicate that we're counting down,

		// allow security code to cancel
		switch(check_security_code())
		{
		case ATTEMPT_SUCCESS:
			door_unlock();
			return STATE_DISARMED;
		case ATTEMPT_FAILURE:
			return STATE_DISARMED;
		}
	}

	return STATE_DISARMED;
}

SystemState run_power_off()
{
	//enter power-off state for MCU

	// returning from this is always an error
	return STATE_ERROR;
}

