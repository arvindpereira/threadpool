#ifndef __TRNG_HH
#define __TRNG_HH
//
//  Project   : misc
//  File      : TRNG.hh
//  Author    : Ronald Kriemann
//  Purpose   : class for a random-number-generator
//

//
//
//
class TRNG
{
protected:
    // state of the RNG (624 == N from TRNG.cc)
    unsigned long   _state[624];
    int             _left;
    unsigned long * _next;

public:
    ////////////////////////////////////////////
    //
    // constructor and destructor
    //

    TRNG ( unsigned long seed = 5489UL ) { if ( seed != 0 ) init( seed ); }

    ////////////////////////////////////////////
    //
    // access rng
    //

    // return random number in the interval [0,max]
    double rand ( double max = 1.0 );

    // abbr. for rand
    double operator () ( double max = 1.0 ) { return rand(max); }

    // initialise rng by single seed (seed must not equal 0)
    void init ( unsigned long seed );

    // initialise rng by array
    void init ( unsigned long init_key[], unsigned long length );

protected:
    // set next state in RNG
    void next_state ();
};

#endif  // __TRANDOM_HH
