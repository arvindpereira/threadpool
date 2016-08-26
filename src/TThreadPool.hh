#ifndef __TTHREADPOOL_HH
#define __TTHREADPOOL_HH
//
//  Project : ThreadPool
//  File    : TThreadPool.hh
//  Author  : Ronald Kriemann
//  Purpose : class for managing a pool of threads
//

#include <iostream>
#include <list>

#include "TThread.hh"

namespace ThreadPool
{

// no specific processor
const int  NO_PROC = -1;

// forward decl. for internal class
class TPoolThr;

//!
//! \class  TPool
//! \brief  implements a thread pool, e.g. takes jobs and
//!         executes them in threads
//!
class TPool
{
    friend class TPoolThr;
    
public:
    ///////////////////////////////////////////
    //!
    //! \class  TJob
    //! \brief  class for a job in the pool
    //!

    class TJob
    {
    protected:
        // @cond
        
        // number of processor this job was assigned to
        const int  _job_no;

        // mutex for synchronisation
        TMutex     _sync_mutex;
        
        // @endcond
        
    public:
        //!
        //! construct job object with \a n as job number
        //!
        TJob ( const int  n = NO_PROC )
                : _job_no(n)
        {}

        //!
        //! destruct job object
        //!
        virtual ~TJob ()
        {
            if ( _sync_mutex.is_locked() )
                std::cerr << "(TJob) destructor : job is still running!" << std::endl;
        }
        
        //!
        //! method to be executed by thread (actual work should be here!)
        //! 
        virtual void run ( void * ptr ) = 0;
        
        //! return assigned job number
        int  job_no () const { return _job_no; }

        //! lock the internal mutex
        void lock   () { _sync_mutex.lock(); }

        //! unlock internal mutex
        void unlock () { _sync_mutex.unlock(); }

        //! return true if if proc-no \a p is local one
        bool on_proc ( const int  p ) const
        {
            return ((p == NO_PROC) || (_job_no == NO_PROC) || (p == _job_no));
        }
    };
    
protected:
    // @cond
    
    // maximum degree of parallelism
    unsigned int             _max_parallel;

    // array of threads, handled by pool
    TPoolThr **              _threads;
    
    // list of idle threads
    std::list< TPoolThr * >  _idle_threads;

    // condition for synchronisation of idle list
    TCondition               _idle_cond;

    // @endcond
    
public:
    ///////////////////////////////////////////////
    //
    // constructor and destructor
    //

    //! construct thread pool with \a max_p threads
    TPool ( const unsigned int  max_p );

    //! wait for all threads to finish and destruct thread pool 
    ~TPool ();

    ///////////////////////////////////////////////
    //
    // access local variables
    //

    //! return number of internal threads, e.g. maximal parallel degree
    unsigned int  max_parallel () const { return _max_parallel; }
    
    ///////////////////////////////////////////////
    //
    // run, stop and synch with job
    //

    //! enqueue \a job in thread pool, e.g. execute \a job by the first freed thread
    //! - \a ptr is an optional argument passed to the "run" method of \a job
    //! - if \a del is true, the job object will be deleted after finishing "run"
    void  run  ( TJob *      job,
                 void *      ptr = NULL,
                 const bool  del = false );

    //! synchronise with \a job, i.e. wait until finished
    void  sync ( TJob * job );

    //! synchronise with all running jobs
    void  sync_all ();

protected:
    ///////////////////////////////////////////////
    //
    // manage pool threads
    //

    //! return idle thread from pool
    TPoolThr * get_idle ();

    //! insert idle thread into pool
    void append_idle ( TPoolThr * t );
};

///////////////////////////////////////////////////
//
// to access the global thread-pool
//
///////////////////////////////////////////////////

//! init global thread_pool with \a max_p threads
void  init      ( const unsigned int   max_p );

//! run \a job in global thread pool with \a ptr passed to job->run()
void  run       ( TPool::TJob *        job,
                  void *               ptr = NULL,
                  const bool           del = false );

//! synchronise with \a job
void  sync      ( TPool::TJob *        job );

//! synchronise with all jobs
void  sync_all  ();

//! finish global thread pool
void  done      ();

}// ThreadPool

#endif  // __TTHREADPOOL_HH
