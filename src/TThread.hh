#ifndef __TTHREAD_HH
#define __TTHREAD_HH
//
//  Project   : ThreadPool
//  File      : TThread.hh
//  Author    : Ronald Kriemann
//  Purpose   : baseclass for a thread-able class
//

#include <cstdio>
#include <pthread.h>

namespace ThreadPool
{

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//!
//! \class  TThread
//! \brief  baseclass for all threaded classes
//!         - defines basic interface
//!
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TThread
{
protected:
    //! @cond
    
    // thread-specific things
    pthread_t  _thread_id;

    // is the thread running or not
    bool       _running;

    // no of thread
    int        _thread_no;
    
    //! @endcond
    
public:
    ////////////////////////////////////////////
    //
    // constructor and destructor
    //

    //! construct thread with thread number \a thread_no
    TThread ( const int thread_no = -1 );

    //! destruct thread, if thread is running, it will be canceled
    virtual ~TThread ();

    ////////////////////////////////////////////
    //
    // access local data
    //

    //! return thread number
    int thread_no () const { return _thread_no; }

    //! set thread number to \a n
    void set_thread_no ( const int  n );

    //! compare if processor number \a p is local one
    bool on_proc ( const int p ) const
    {
        return ((p == -1) || (_thread_no == -1) || (p == _thread_no));
    }

    ////////////////////////////////////////////
    //
    // user-interface
    //
    
    //! actual method to be executed by thread
    virtual void run () = 0;

    ////////////////////////////////////////////
    //
    // thread management
    //

    //! create thread (actually start it);
    //!   - if \a detached is true, the thread will be started in detached mode,
    //!     e.g. can not be joined,
    //!   - if \a sscope is true, the thread is started in system scope, e.g.
    //!     the thread competes for resources with all other threads of all
    //!     processes on the system; if \a sscope is false, the competition is
    //!     only process local
    void create ( const bool  detached = false,
                  const bool  sscope   = false );

    //! detach thread
    void detach ();
    
    //! synchronise with thread (wait until finished)
    void join   ();

    //! request cancellation of thread
    void cancel ();

protected:
    //! @cond
    
    ////////////////////////////////////////////
    //
    // functions to be called by a thread itself
    //
    
    //! terminate thread
    void exit   ();

    //! put thread to sleep for <sec> seconds
    void sleep  ( const double sec );

public:
    
    ////////////////////////////////////////////
    //
    // internally used, but public functions
    //
    
    //! resets running-status (used in _run_proc, see TThread.cc)
    void reset_running () { _running = false; }
    
    //! @endcond
};



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//
// wrapper for pthread_mutex
//
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TMutex
{
protected:
    //! @cond

    // the mutex itself and the mutex-attr
    pthread_mutex_t      _mutex;
    pthread_mutexattr_t  _mutex_attr;

    //! @endcond
    
public:
    /////////////////////////////////////////////////
    //
    // constructor and destructor
    //

    //! construct unlocked mutex
    TMutex ()
    {
        pthread_mutexattr_init( & _mutex_attr );
        pthread_mutex_init( & _mutex, & _mutex_attr );
    }

    //! dtor
    ~TMutex ()
    {
        pthread_mutex_destroy( & _mutex );
        pthread_mutexattr_destroy( & _mutex_attr );
    }

    /////////////////////////////////////////////////
    //
    // usual behavior of a mutex
    //

    //! lock mutex
    void  lock    () { pthread_mutex_lock(   & _mutex ); }

    //! unlock mutex
    void  unlock  () { pthread_mutex_unlock( & _mutex ); }

    //! return true if mutex is locked and false, otherwise
    bool is_locked ()
    {
        if ( pthread_mutex_trylock( & _mutex ) != 0 )
            return true;
        else
        {
            unlock();
            return false;
        }// else
    }
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//!
//! \class  TScopedLock
//! \brief  Provides automatic lock and unlock for mutices.
//!
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TScopedLock
{
private:
    //! @cond

    //! given mutex for locking
    TMutex *  _mutex;

    // prevent copy operations
    TScopedLock ( TScopedLock & );
    void operator = ( TScopedLock & );

    //! @endcond

public:
    //! ctor: lock mutex
    explicit TScopedLock ( TMutex &  m )
            : _mutex( & m )
    {
        _mutex->lock();
    }

    //! dtor: unlock mutex
    ~TScopedLock ()
    {
        _mutex->unlock();
    }
};

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//!
//! \class TCondition
//! \brief class for a condition variable
//!        - derived from mutex to allow locking of condition
//!          to inspect or modify the predicate
//!
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

class TCondition : public TMutex
{
private:
    //! @cond
    
    // our condition variable
    pthread_cond_t  _cond;

    //! @endcond
public:
    /////////////////////////////////////////////////
    //
    // constructor and destructor
    //

    //! ctor
    TCondition  () { pthread_cond_init(    & _cond, NULL ); }

    //! dtor
    ~TCondition () { pthread_cond_destroy( & _cond ); }

    /////////////////////////////////////////////////
    //
    // condition variable related methods
    //

    //! wait for signal to arrive
    void wait      () { pthread_cond_wait( & _cond, & _mutex ); }

    //! restart one of the threads, waiting on the cond. variable
    void signal    () { pthread_cond_signal( & _cond ); }

    //! restart all waiting threads
    void broadcast () { pthread_cond_broadcast( & _cond ); }
};

}// namespace ThreadPool

#endif  // __TTHREAD_HH
