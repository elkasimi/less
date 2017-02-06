#pragma once

class Board;
class Timer
{
public:
    Timer( );
    ~Timer( );

    void reset( );
    void stop( );
    double get_delta_time( );

    void set_alarm( const int32_t microseconds );
    void clear_alarm( );
    bool is_time_over( ) const;

    static double get_max_time( const Board& board );
    static double get_total_time( );
    static double get_max_total_time( );

private:
    static int32_t s_total_time;
    static int32_t s_max_total_time;

    using SystemClock = std::chrono::system_clock;
    using TimePoint = SystemClock::time_point;

    TimePoint m_start;
    TimePoint m_end;
    int32_t m_delta_time;
    TimePoint m_alarm_start_time;
    int32_t m_alarm_delta_time;
    bool m_alarm_set;
    mutable bool m_time_over;
};
