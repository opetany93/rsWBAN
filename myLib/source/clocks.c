#include "clocks.h"

#include "nrf.h"
#include "nrf_clock.h"
#include "nrf_rtc.h"

#include "mytypes.h"
#include "mydefinitions.h"

// =======================================================================================
inline int8_t startLFCLK(void)
{
	if( !isLFCLKstable() )
	{
		// Starts the internal LFCLK XTAL oscillator for RTC
		nrf_clock_lf_src_set(NRF_CLOCK_LFCLK_Xtal);
		nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
		nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);

		while( !isLFCLKstable() );
		
		nrf_rtc_task_trigger(RTC0, NRF_RTC_TASK_START);  			// added to avoid anomaly (errata [20])
		
		return 0;
	}
	else
	{
		return -1;
	}
}

bool isLFCLKstable(void)
{
	if ( nrf_clock_lf_start_task_status_get() )
	{
		nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
		return true;
	}
	else
	{
		return false;
	}
}

// =======================================================================================
inline void stopHFCLK()
{
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTOP);

	while ( !isHFCLKstopped() )
		;
}

bool isHFCLKstopped()
{
	if ( nrf_clock_hf_is_running(NRF_CLOCK_HFCLK_HIGH_ACCURACY) )
	{
		return false;
	}
	else
	{
		return true;
	}
}

inline int8_t startHFCLK(void)
{
	if ( !isHFCLKstable() )
	{
		nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
		nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
		
		while ( !isHFCLKstable() )
			;
		
		return 0;
	}
	else
	{
		return -1;
	}
}

bool isHFCLKstable(void)
{
	if ( nrf_clock_hf_is_running(NRF_CLOCK_HFCLK_HIGH_ACCURACY) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

