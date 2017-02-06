#include "Common.h"

#include "RandomNumberGenerator.h"

namespace
{
std::default_random_engine engine{};
}

void
RandomNumberGenerator::randomize( )
{
    std::random_device rd{};
    auto s = rd( );
    engine.seed( s );
    std::cerr << "seed=" << s << "\n";
}

uint32_t
RandomNumberGenerator::pick( const uint32_t limit_value )
{
    using IntDistribution = std::uniform_int_distribution< uint32_t >;

    IntDistribution distribution{};
    const uint32_t min_value = 0u;
    const uint32_t max_value
        = limit_value == 0u ? std::numeric_limits< uint32_t >::max( ) : limit_value - 1u;
    const IntDistribution::param_type parameter{min_value, max_value};

    return distribution( engine, parameter );
}

double
RandomNumberGenerator::pick( const double limit_value )
{
    using RealDistribution = std::uniform_real_distribution< double >;

    RealDistribution distribution{};
    const RealDistribution::param_type parameter{0.0, limit_value};

    return distribution( engine, parameter );
}
