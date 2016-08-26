#ifndef __TIMER_HH
#define __TIMER_HH
//
//  Project : misc
//  File    : timer.hh
//  Author  : Ronald Kriemann
//  Purpose : class for a timer (for speed tests)
//

#include <sys/time.h>
#include <sys/resource.h>

#ifdef SUNOS
#include <sys/lwp.h>
#endif
    
#include <unistd.h>

#include <iostream>

// type for different timer-measurment
typedef enum { REAL_TIME, CPU_TIME }  timetype_t;

//
// the timer class
//
class TTimer 
{
public:
    
protected:
    // start/stop times
    double          _start, _stop;

    // temp structs
    struct timeval  _timeval_data;
#ifdef SUNOS
    struct lwpinfo  _lwpinfo_data;
#else
    struct rusage   _rusage_data;
#endif
    
    // what kind of time we should stop
    timetype_t      _type;
    
public:
    /////////////////////////////////////////////////
    //
    // constructor
    //

    TTimer ( timetype_t type = CPU_TIME ) : _start(0), _stop(0), _type(type) {}
    
    /////////////////////////////////////////////////
    //
    // compute times
    //
    
	// sets first/second timer (cpu- or real-time by _real-field)
    TTimer & start    () { _stop = _start = system_time(); return *this; }
    TTimer & stop     () { _stop =          system_time(); return *this; }

	// returns time between start and end in seconds
	float diff () const { return _stop - _start; }

    // get time of system (usertime or real)
    double system_time ();
    
    /////////////////////////////////////////////////
    //
    // output
    //

	// stream output (only the time)
	friend std::ostream & operator << ( std::ostream & os, const TTimer & timer );
};

#endif  // __TIMER_HH
