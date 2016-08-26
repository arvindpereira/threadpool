//
//  Project   : misc
//  File      : timer.cc
//  Author    : Ronald Kriemann
//  Purpose   : class for a timer (for speed tests)
//

#include <sys/times.h>
#include <limits.h>

#include <cmath>

#include "TTimer.hh"

//
// get time of system (usertime or real)
//
double
TTimer::system_time ()
{
    double  sec, usec;
    
    switch ( _type )
    {
        case CPU_TIME :
#ifdef SUNOS
            _lwp_info( & _lwpinfo_data );
            sec   = _lwpinfo_data.lwp_utime.tv_sec  + _lwpinfo_data.lwp_stime.tv_sec;
            usec  = _lwpinfo_data.lwp_utime.tv_nsec + _lwpinfo_data.lwp_stime.tv_nsec;
            usec /= 1000;
#else
            getrusage( RUSAGE_SELF, & _rusage_data );
            sec  = _rusage_data.ru_utime.tv_sec  + _rusage_data.ru_stime.tv_sec;
            usec = _rusage_data.ru_utime.tv_usec + _rusage_data.ru_stime.tv_usec;
#endif
                
            break;

        case REAL_TIME :
        default :
            gettimeofday( & _timeval_data, NULL );

            sec  = _timeval_data.tv_sec;
            usec = _timeval_data.tv_usec;

            break;
    }// switch

    return sec + (usec * 1e-6);
}

//
// stream output (only the time)
//
std::ostream & 
operator << ( std::ostream & os, const TTimer & timer )
{
    double  time;
	long    seconds, mseconds;

    time     = 0 > timer.diff() ? 0 : timer.diff();
    seconds  = long( floor( time ) );
    mseconds = long(floor( (time - double(seconds)) * 1000.0 + 0.5 ));
        
	os << seconds << ',';
    
    if (mseconds < 10)        os << "00";
    else if (mseconds < 100)  os << "0";

    os << mseconds << "s";

	return os;
}
