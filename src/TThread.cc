//
//  Project   : ThreadPool
//  File      : TThread.cc
//  Author    : Ronald Kriemann
//  Purpose   : baseclass for a thread-able class
//

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sched.h>
#include <string.h>

#include <iostream>
#include <cmath>

#include "TThread.hh"

namespace ThreadPool
{

//
// routine to call TThread::run() method
//
extern "C"
void *
_run_thread ( void *arg )
{
    if (arg != NULL)
    {
        ((TThread*) arg)->run();
        ((TThread*) arg)->reset_running();
    }// if

    return NULL;
}

////////////////////////////////////////////
//
// constructor and destructor
//
    
TThread::TThread ( const int athread_no )
        : _running( false ), _thread_no(athread_no)
{
}

TThread::~TThread ()
{
    // request cancellation of the thread if running
    if ( _running )
        cancel();
}

////////////////////////////////////////////
//
// access local data
//

void
TThread::set_thread_no ( const int  no )
{
    _thread_no = no;
}
    
////////////////////////////////////////////
//
// thread management
//

//
// create thread (actually start it)
//
void 
TThread::create ( const bool  detached,
                  const bool  sscope )
{
    if ( ! _running )
    {
        int             status;
        pthread_attr_t  thread_attr;
        
        if ((status = pthread_attr_init( & thread_attr )) != 0)
        {
            std::cerr << "(TThread) create : pthread_attr_init ("
                      << strerror( status ) << ")" << std::endl;
            return;
        }// if

        if ( detached )
        {
            // detache created thread from calling thread
            if ((status = pthread_attr_setdetachstate( & thread_attr,
                                                       PTHREAD_CREATE_DETACHED )) != 0)
            {
                std::cerr << "(TThread) create : pthread_attr_setdetachstate ("
                          << strerror( status ) << ")" << std::endl;
                return;
            }// if
        }// if

        if ( sscope )
        {
            // use system-wide scheduling for thread
            if ((status = pthread_attr_setscope( & thread_attr, PTHREAD_SCOPE_SYSTEM )) != 0 )
            {
                std::cerr << "(TThread) create : pthread_attr_setscope ("
                          << strerror( status ) << ")" << std::endl;
                return;
            }// if
        }// if
            
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) && defined(SUNOS)
        //
        // adjust thread-scheduling for Solaris
        //
        
        struct sched_param  t_param;
        
        t_param.sched_priority = sched_get_priority_min( SCHED_RR );
        
        if ((status = pthread_attr_setschedpolicy(  & thread_attr, SCHED_RR )) != 0)
            std::cerr << "(TThread) create : pthread_attr_setschedpolicy ("
                      << strerror( status ) << ")" << std::endl;
        
        if ((status = pthread_attr_setschedparam(   & thread_attr, & t_param )) != 0)
            std::cerr << "(TThread) create : pthread_attr_setschedparam ("
                      << strerror( status ) << ")" << std::endl;

        if ((status = pthread_attr_setinheritsched( & thread_attr, PTHREAD_EXPLICIT_SCHED )) != 0)
            std::cerr << "(TThread) create : pthread_attr_setinheritsched ("
                      << strerror( status ) << ")" << std::endl;
#endif

#ifdef HPUX
        // on HP-UX we increase the stack-size for a stable behaviour
        // (need much memory for this !!!)
        pthread_attr_setstacksize( & thread_attr, 32 * 1024 * 1024 );
#endif
        
        if ((status = pthread_create( & _thread_id, & thread_attr, _run_thread, this ) != 0))
            std::cerr << "(TThread) create : pthread_create ("
                      << strerror( status ) << ")" << std::endl;
        else
            _running = true;

        // remove attribute
        pthread_attr_destroy( & thread_attr );
    }// if
    else
        std::cout << "(TThread) create : thread is already running" << std::endl;
}

//
// detach thread
//
void 
TThread::detach ()
{
    if ( _running )
    {
        int status;
        
        // detach thread
        if ((status = pthread_detach( _thread_id )) != 0)
            std::cerr << "(TThread) detach : pthread_detach ("
                      << strerror( status ) << ")" << std::endl;
    }// if
}

//
// synchronise with thread (wait until finished)
//
void 
TThread::join ()
{
    if ( _running )
    {
        int status;
    
        // wait for thread to finish
        if ((status = pthread_join( _thread_id, NULL )) != 0)
            std::cerr << "(TThread) join : pthread_join ("
                      << strerror( status ) << ")" << std::endl;

        _running = false;
    }// if
}

//
// request cancellation of thread
//
void 
TThread::cancel ()
{
    if ( _running )
    {
        int status;
        
        if ((status = pthread_cancel( _thread_id )) != 0)
            std::cerr << "(TThread) cancel : pthread_cancel ("
                      << strerror( status ) << ")" << std::endl;
    }// if
}

////////////////////////////////////////////
//
// functions to be called by a thread itself
//

//
// terminate thread
//
void 
TThread::exit ()
{
    if ( _running && (pthread_self() == _thread_id))
    {
        void  * ret_val = NULL;

        pthread_exit( ret_val );
        
        _running = false;
    }// if
}

//
// put thread to sleep (milli + nano seconds)
//
void
TThread::sleep ( const double sec )
{
    if ( _running )
    {
        struct timespec  interval;

        if ( sec <= 0.0 )
        {
            interval.tv_sec  = 0;
            interval.tv_nsec = 0;
        }// if
        else
        {
            interval.tv_sec  = time_t( std::floor( sec ) );
            interval.tv_nsec = long( (sec - interval.tv_sec) * 1e6 );
        }// else
        
        nanosleep( & interval, 0 );
    }// if
}

}// namespace ThreadPool
