#include "clocks.h"

#include "nrf.h"

#include "mytypes.h"
#include "mydefinitions.h"

// =======================================================================================
int8_t startLFCLK(void)
{
	if( !isLFCLKstable() )
	{
		// Starts the internal LFCLK XTAL oscillator for RTC
		CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
		CLOCK->EVENTS_LFCLKSTARTED = 0;
		CLOCK->TASKS_LFCLKSTART = 1;
		
		while( !isLFCLKstable() );
		
		RTC0->TASKS_STOP = 0;													// added to avoid anomaly (errata [20])
		
		return 0;
	}
	else
	{
		return -1;
	}
}

uint8_t isLFCLKstable(void)
{
	uint32_t stable = (uint32_t)(CLOCK_LFCLKSTAT_STATE_Running << CLOCK_LFCLKSTAT_STATE_Pos) | (CLOCK_LFCLKSTAT_SRC_Xtal << CLOCK_LFCLKSTAT_SRC_Pos);
	
	if ( CLOCK->LFCLKSTAT == stable )
	{
		CLOCK->EVENTS_LFCLKSTARTED = 0;
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// =======================================================================================
void stopHFCLK()
{
	CLOCK->TASKS_HFCLKSTOP = 1U;

	while ( !isHFCLKstopped() )
		;
}

uint8_t isHFCLKstopped()
{
	uint32_t stat = (uint32_t)(CLOCK_HFCLKSTAT_STATE_NotRunning << CLOCK_HFCLKSTAT_STATE_Pos) | (CLOCK_HFCLKSTAT_SRC_Xtal << CLOCK_HFCLKSTAT_SRC_Pos);

	if ( CLOCK->HFCLKSTAT == stat )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

int8_t startHFCLK(void)
{
	if ( !isHFCLKstable() )
	{
		CLOCK->EVENTS_HFCLKSTARTED = 0;
		CLOCK->TASKS_HFCLKSTART = 1U;
		
		while ( !isHFCLKstable() )
			;
		
		return 0;
	}
	else
	{
		return -1;
	}
}

uint8_t isHFCLKstable(void)
{
	uint32_t stable = (uint32_t)(CLOCK_HFCLKSTAT_STATE_Running << CLOCK_HFCLKSTAT_STATE_Pos) | (CLOCK_HFCLKSTAT_SRC_Xtal << CLOCK_HFCLKSTAT_SRC_Pos);
	
	if ( CLOCK->HFCLKSTAT == stable )
	{
		CLOCK->EVENTS_HFCLKSTARTED = 0;
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

