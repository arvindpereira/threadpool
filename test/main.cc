
#include <cstdlib>
#include <cmath>
#include <vector>

#include "TThreadPool.hh"
#include "TTimer.hh"
#include "TRNG.hh"

//
// for some benchmarking
//

int job_number = 0;

class TBenchJob : public ThreadPool::TPool::TJob
{
protected:
    int   _size;
    
public:
    TBenchJob ( int i, int s ) : ThreadPool::TPool::TJob( i ), _size(s) {}

    virtual void run ( void * )
    {
        std::vector< double > matrix( _size * _size );
        
        for ( int i = 0; i < _size; i++ )
        {
            for ( int j = 0; j < _size; j++ )
            {
                matrix[ (i*_size) + j ] = std::sin( double(j) * M_PI * std::cos( double(i) ));
            }// for
        }// for
    }
};

#define MAX_SIZE  1000
#define MAX_RAND  500

void
recursion ( int level, TRNG & rng )
{
    if ( level == 0 )
    {
        TBenchJob * job = new TBenchJob( -1, int(rng.rand( MAX_RAND )) + MAX_SIZE );

        ThreadPool::run( job, NULL, true );
    }// if
    else
    {
        recursion( level-1, rng );
        recursion( level-1, rng );
        recursion( level-1, rng );
        recursion( level-1, rng );
    }// else
}

void
bench1 ( int argc, char ** argv )
{
    TRNG  rng;
    int   i = 1;
    int   thr_count = 16;
    int   rec_depth = 6;
    
    if ( argc > 1 ) thr_count = atoi( argv[1] );
    if ( argc > 2 ) rec_depth = atoi( argv[2] );

    for ( int j = 0; j < rec_depth; j++ )
        i *= 4;
    
    ThreadPool::init( thr_count );

    std::cout << "executing " << i << " jobs using " << thr_count << " thread(s)" << std::endl;
    
    TTimer  timer( REAL_TIME );

    timer.start();
    
    recursion( rec_depth, rng );
    
    ThreadPool::sync_all();

    timer.stop();
    std::cout << "time for recursion = " << timer << std::endl;

    ThreadPool::done();
}

class TBench2Job : public ThreadPool::TPool::TJob
{
public:
    TBench2Job ( int i ) : ThreadPool::TPool::TJob( i ) {}

    virtual void run ( void * )
    {
        // do nothing
    }
};

class TBench2Thr : public ThreadPool::TThread
{
public:
    TBench2Thr ( int i ) : TThread( i ) {}

    virtual void run ()
    {
        // do nothing
    }
};

void
bench2 ( int argc, char ** argv )
{
    int  max_jobs = 500000;

    ThreadPool::init( 4 );

    TTimer  timer( REAL_TIME );

    timer.start();
    
    for ( int i = 0; i  < max_jobs; i++ )
    {
        TBench2Job  * job = new TBench2Job( i );
        delete job;
    }

    timer.stop();
    std::cout << "time to create jobs = " << timer << std::endl;

    timer.start();
    
    for ( int i = 0; i  < max_jobs; i++ )
    {
        TBench2Thr  * job = new TBench2Thr( i );
        delete job;
    }

    timer.stop();
    std::cout << "time to create threads = " << timer << std::endl;

    timer.start();
    
    for ( int i = 0; i  < max_jobs; i++ )
    {
        TBench2Job  * job = new TBench2Job( i );

        ThreadPool::run( job );
        ThreadPool::sync( job );

        delete job;
    }

    timer.stop();
    std::cout << "time for thread pool = " << timer << std::endl;

    timer.start();
    
    for ( int i = 0; i  < max_jobs; i++ )
    {
        TBench2Thr  * job = new TBench2Thr( i );

        job->create( false, false );
        job->join();
        
        delete job;
    }

    timer.stop();

    std::cout << "time for lwp-threads = " << timer << std::endl;
    timer.start();
    
    for ( int i = 0; i  < max_jobs; i++ )
    {
        TBench2Thr  * job = new TBench2Thr( i );

        job->create( false, true );
        job->join();
        
        delete job;
    }

    timer.stop();
    std::cout << "time for hwp-threads = " << timer << std::endl;

    ThreadPool::done();
}

int
main ( int argc, char ** argv )
{
    // bench1( argc, argv );
    bench2( argc, argv );
}
