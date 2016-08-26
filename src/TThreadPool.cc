//
//  Project : ThreadPool
//  File    : TThreadPool.cc
//  Author  : Ronald Kriemann
//  Purpose : class for managing a pool of threads
//

#include <pthread.h>

#include "TThreadPool.hh"

namespace ThreadPool
{

namespace
{

//
// set to one to enable sequential execution, e.g. for debugging
//
#define THR_SEQUENTIAL  0

//
// global thread-pool
//
TPool * thread_pool = NULL;

}// namespace anonymous

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// thread handled by threadpool
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// thread handled by threadpool
//

class TPoolThr : public TThread
{
protected:
    // pool we are in
    TPool *        _pool;
    
    // job to run and data for it
    TPool::TJob *  _job;
    void *         _data_ptr;

    // should the job be deleted upon completion
    bool           _del_job;
    
    // condition for job-waiting
    TCondition     _work_cond;
    
    // indicates end-of-thread
    bool           _end;

    // mutex for preventing premature deletion
    TMutex         _del_mutex;
    
public:
    //
    // constructor
    //
    TPoolThr ( const int n, TPool * p )
            : TThread(n), _pool(p), _job(NULL), _data_ptr(NULL), _del_job(false), _end(false)
    {}
    
    ~TPoolThr () {}
    
    //
    // parallel running method
    //
    void run ()
    {
        TScopedLock  del_lock( _del_mutex );

        while ( ! _end )
        {
            //
            // append thread to idle-list and wait for work
            //

            _pool->append_idle( this );

            {
                TScopedLock  work_lock( _work_cond );
            
                while (( _job == NULL ) && ! _end )
                    _work_cond.wait();
            }
        
            //
            // look if we really have a job to do
            // and handle it
            //

            if ( _job != NULL )
            {
                // execute job
                _job->run( _data_ptr );
                _job->unlock();
            
                if ( _del_job )
                    delete _job;

                // reset data
                TScopedLock  work_lock( _work_cond );

                _job      = NULL;
                _data_ptr = NULL;
            }// if
        }// while
    }

    //
    // set and run job with optional data
    //
    void run_job  ( TPool::TJob * j, void * p, const bool del = false )
    {
        TScopedLock  lock( _work_cond );
        
        _job      = j;
        _data_ptr = p;
        _del_job  = del;
        
        _work_cond.signal();
    }

    //
    // give access to delete mutex
    //
    
    TMutex & del_mutex  ()
    {
        return _del_mutex;
    }

    //
    // quit thread (reset data and wake up)
    //
    
    void quit ()
    {
        TScopedLock  lock( _work_cond );
        
        _end      = true;
        _job      = NULL;
        _data_ptr = NULL;
        
        _work_cond.signal();
    }
};
    
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// ThreadPool - implementation
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//
// constructor and destructor
//

TPool::TPool ( const unsigned int  max_p )
{
    //
    // create max_p threads for pool
    //

    _max_parallel = max_p;

    _threads = new TPoolThr*[ _max_parallel ];

    if ( _threads == NULL )
    {
        _max_parallel = 0;
        std::cerr << "(TPool) TPool : could not allocate thread array" << std::endl;
    }// if
    
    for ( unsigned int  i = 0; i < _max_parallel; i++ )
    {
        _threads[i] = new TPoolThr( i, this );

        if ( _threads == NULL )
            std::cerr << "(TPool) TPool : could not allocate thread" << std::endl;
        else
            _threads[i]->create( true, true );
    }// for

    // tell the scheduling system, how many threads to expect
    // (commented out since not needed on most systems)
//     if ( pthread_setconcurrency( _max_parallel + pthread_getconcurrency() ) != 0 )
//         std::cerr << "(TPool) TPool : pthread_setconcurrency ("
//                   << strerror( status ) << ")" << std::endl;
}

TPool::~TPool ()
{
    // wait till all threads have finished
    sync_all();
    
    // finish all thread
    for ( unsigned int  i = 0; i < _max_parallel; i++ )
        _threads[i]->quit();
    
    // cancel and delete all threads (not really safe !)
    for ( unsigned int  i = 0; i < _max_parallel; i++ )
    {
        _threads[i]->del_mutex().lock();
        delete _threads[i];
    }// for

    delete[] _threads;
}

///////////////////////////////////////////////
//
// run, stop and synch with job
//

void
TPool::run ( TPool::TJob * job, void * ptr, const bool del )
{
    if ( job == NULL )
        return;

#if THR_SEQUENTIAL == 1
    //
    // run in calling thread
    //
    
    job->run( ptr );

    if ( del )
        delete job;
    
#else
    //
    // run in parallel thread
    //

    TPoolThr * thr = get_idle();
    
    // lock job for synchronisation
    job->lock();

    // attach job to thread
    thr->run_job( job, ptr, del );
#endif
}

//
// wait until <job> was executed
//
void
TPool::sync ( TJob * job )
{
    if ( job == NULL )
        return;
    
    job->lock();
    job->unlock();
}

//
// wait until all jobs have been executed
//
void
TPool::sync_all ()
{
    while ( true )
    {
        {
            TScopedLock  lock( _idle_cond );
        
            // wait until next thread becomes idle
            if ( _idle_threads.size() < _max_parallel )
                _idle_cond.wait();
            else
            {
                break;
            }// else
        }
    }// while
}

///////////////////////////////////////////////
//
// manage pool threads
//

//
// return idle thread form pool
//
TPoolThr *
TPool::get_idle ()
{
    while ( true )
    {
        //
        // wait for an idle thread
        //

        TScopedLock  lock( _idle_cond );
        
        while ( _idle_threads.empty() )
            _idle_cond.wait();

        //
        // get first idle thread
        //
        
        if ( ! _idle_threads.empty() )
        {
            TPoolThr * t = _idle_threads.front();

            _idle_threads.pop_front();
            
            return t;
        }// if
    }// while
}

//
// append recently finished thread to idle list
//
void
TPool::append_idle ( TPoolThr * t )
{
    //
    // CONSISTENCY CHECK: if given thread is already in list
    //
    
    TScopedLock  lock( _idle_cond );

    for ( std::list< TPoolThr * >::iterator  iter = _idle_threads.begin();
          iter != _idle_threads.end();
          ++iter )
    {
        if ( (*iter) == t )
        {
            return;
        }// if
    }// while
    
    _idle_threads.push_back( t );

    // wake a blocked thread for job execution
    _idle_cond.signal();
}

///////////////////////////////////////////////////
//
// to access global thread-pool
//
///////////////////////////////////////////////////

//
// init global thread_pool
//
void
init ( const unsigned int  max_p )
{
    if ( thread_pool != NULL )
        delete thread_pool;
    
    if ((thread_pool = new TPool( max_p )) == NULL)
        std::cerr << "(init_thread_pool) could not allocate thread pool" << std::endl;
}

//
// run job
//
void
run ( TPool::TJob * job, void * ptr, const bool del )
{
    if ( job == NULL )
        return;
    
    thread_pool->run( job, ptr, del );
}

//
// synchronise with specific job
//
void
sync ( TPool::TJob * job )
{
    thread_pool->sync( job );
}

//
// synchronise with all jobs
//
void
sync_all ()
{
    thread_pool->sync_all();
}

//
// finish thread pool
//
void
done ()
{
    delete thread_pool;
}

}// namespace ThreadPool
