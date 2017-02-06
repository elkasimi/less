#pragma once

class RandomNumberGenerator
{
public:
    static void randomize( );
    static uint32_t pick( const uint32_t limit_value = 0u );
    static double pick( const double limit_value );
};
