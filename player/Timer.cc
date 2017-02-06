#include "Common.h"

#include "Board.h"
#include "Timer.h"

int32_t Timer::s_total_time = 0;
int32_t Timer::s_max_total_time = 30000000;

using std::chrono::microseconds;
using std::chrono::duration_cast;

Timer::Timer( )
    : m_start( SystemClock::now( ) )
    , m_delta_time( 0 )
    , m_alarm_delta_time( 0 )
    , m_alarm_set( false )
    , m_time_over( false )
{
}

Timer::~Timer( )
{
}

void
Timer::reset( )
{
    m_start = SystemClock::now( );
}

void
Timer::stop( )
{
    m_end = SystemClock::now( );
    const auto dur = duration_cast< microseconds >( m_end - m_start );
    m_delta_time = (int32_t)dur.count( );
    s_total_time += m_delta_time;
}

double
Timer::get_delta_time( )
{
    return 1e-6 * m_delta_time;
}

double
Timer::get_max_time( const Board& board )
{
    const int32_t remaining_turns = 21 - ( board.get_turns( ) / 4 );
    const int32_t max_turn_time = ( s_max_total_time - s_total_time ) / remaining_turns;

    return 1e-6 * max_turn_time;
}

double
Timer::get_total_time( )
{
    return 1e-6 * s_total_time;
}

double
Timer::get_max_total_time( )
{
    return 1e-6 * s_max_total_time;
}

void
Timer::set_alarm( const int32_t microseconds )
{
    m_alarm_start_time = SystemClock::now( );
    m_alarm_delta_time = microseconds;
    m_time_over = false;
    m_alarm_set = true;
}

void
Timer::clear_alarm( )
{
    m_alarm_set = false;
}

bool
Timer::is_time_over( ) const
{
    if ( not m_time_over )
    {
        auto now = SystemClock::now( );
        auto dur = duration_cast< microseconds >( now - m_alarm_start_time );
        auto delta_time = dur.count( );
        m_time_over = delta_time >= m_alarm_delta_time;
    }

    return m_time_over;
}
