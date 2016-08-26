//
//  Project   : misc
//  File      : TRNG.cc
//  Author    : Ronald Kriemann
//  Purpose   : class for a random-number-generator
//

#include "TRNG.hh"

/* A C-program for MT19937, with initialization improved 2002/2/10.*/
/* Coded by Takuji Nishimura and Makoto Matsumoto.                 */
/* This is a faster version by taking Shawn Cokus's optimization,  */
/* Matthe Bellew's simplification, Isaku Wada's real version.      */

/* Before using, initialize the state by using init_genrand(seed)  */
/* or init_by_array(init_key, key_length).                         */

/* This library is free software.                                  */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */

/* Copyright (C) 1997, 2002 Makoto Matsumoto and Takuji Nishimura. */
/* Any feedback is very welcome.                                   */
/* http://www.math.keio.ac.jp/matumoto/emt.html                    */
/* email: matumoto@math.keio.ac.jp                                 */

//
// some defines/numbers wich are used in the RNG
//

#define N            624
#define M            397
#define MATRIX_A     0x9908b0dfUL        // constant vector a
#define UMASK        0x80000000UL        // most significant w-r bits
#define LMASK        0x7fffffffUL        // least significant r bits
#define MIXBITS(u,v) ( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v)   ((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

typedef  unsigned long  ulong;

////////////////////////////////////////////
//
// access rng
//

//
// return random number between 0 and max
//
double
TRNG::rand ( double max )
{
    ulong y;

    if ( --_left == 0 )
        next_state();
    
    y = *_next++;

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    /* divided by 2^32-1 */ 
    return max * double( y ) * (1.0/4294967295.0); 
}

//
// initialise rng
//
void
TRNG::init ( ulong seed )
{
    _state[0] = seed & 0xffffffffUL;

    for ( int j = 1; j < N; j++ )
    {
        _state[j] = (1812433253UL * (_state[j-1] ^ (_state[j-1] >> 30)) + j);
        
        // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
        // In the previous versions, MSBs of the seed affect
        // only MSBs of the array state[].                 
        // 2002/01/09 modified by Makoto Matsumoto         
        _state[j] &= 0xffffffffUL;  // for >32 bit machines 
    }// for
    
    _left = 1;
}

//
// initialise rng by array
//
void
TRNG::init ( ulong init_key[], ulong key_length )
{
    int i, j, k;

    init( 19650218UL );

    i = 1;
    j = 0;
    k = int(N > key_length ? N : key_length);
    
    for ( ; k; k-- )
    {
        _state[i]  = ((_state[i] ^ ((_state[i-1] ^ (_state[i-1] >> 30)) * 1664525UL))
                      + init_key[j] + j); // non linear
        _state[i] &= 0xffffffffUL;        // for WORDSIZE > 32 machines
        
        i++;
        j++;
        
        if (i >= N)
        {
            _state[0] = _state[N-1];
            i        = 1;
        }// if
        
        if ( ulong(j) >= key_length )
            j = 0;
    }// for
    
    for ( k = N-1; k; k-- )
    {
        _state[i]  = ((_state[i] ^ ((_state[i-1] ^ (_state[i-1] >> 30)) * 1566083941UL))
                      - i);              // non linear
        _state[i] &= 0xffffffffUL;       // for WORDSIZE > 32 machines
        
        i++;
        
        if (i >= N)
        {
            _state[0] = _state[N-1];
            i        = 1;
        }// if
    }// for

    _state[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
    _left     = 1;
}

//
// set next state in RNG
//
void
TRNG::next_state ()
{
    ulong * p = _state;
    int     j;

    _left = N;
    _next = _state;
    
    for ( j = N-M+1; --j; p++ ) 
        *p = p[M] ^ TWIST(p[0], p[1]);

    for ( j = M; --j; p++ ) 
        *p = p[M-N] ^ TWIST(p[0], p[1]);

    *p = p[M-N] ^ TWIST(p[0], _state[0]);
}
