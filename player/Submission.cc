
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define N 8
#define NXN 64
#define NXNX8 512
#define NIL_MOVE ( (int32_t)-1 )
#define INVALID_MOVE ( (int32_t)-2 )
#define CREATE_MOVE( from, to ) ( ( from ) | ( ( to ) << 6 ) )
#define OO 1000000000
#define MAX_MOVES 17

using Move = int32_t;
using Action = std::vector< Move >;

class RandomNumberGenerator
{
public:
    static void randomize( );
    static uint32_t pick( const uint32_t limit_value = 0u );
    static double pick( const double limit_value );
};

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

struct Entry
{
    enum Type : uint8_t
    {
        LOWER,
        EXACT,
        UPPER
    };

    Entry( )
        : lock( 0u )
        , depth( 0 )
        , type( EXACT )
        , value( 0.0 )
        , move( INVALID_MOVE )
    {
    }

    uint32_t lock;
    int32_t depth;
    Type type;
    double value;
    Move move;
};

class Board;
class TranpositionTable
{
public:
    TranpositionTable( );
    ~TranpositionTable( );

    const Entry* find( const Board& board );
    void store( const Board& board,
                const int32_t depth,
                const Entry::Type type,
                const double value,
                const Move move );

private:
    std::vector< Entry > m_buckets;
};

class Board
{
public:
    enum Player : uint8_t
    {
        YELLOW = 0,
        BLACK = 1,
        WHITE = 2,
        RED = 3
    };

    enum MoveType : uint8_t
    {
        RIGHT_1 = 0,
        RIGHT_2 = 1,
        LEFT_1 = 2,
        LEFT_2 = 3,
        UP_1 = 4,
        UP_2 = 5,
        DOWN_1 = 6,
        DOWN_2 = 7
    };

    Board( );
    Board( const Player player, const uint64_t bitmask );
    Board( const Player player, const uint64_t bitmasks[ 4 ] );

    ~Board( );

    static void init_data( const std::string& walls );
    static void pre_compute( );

    Player get_player( ) const;
    int32_t do_move( const Move move );
    void do_action( const Action& action );
    void enable_solo_mode( const Player player );
    bool end_game( ) const;
    double get_mobility( const Player player ) const;
    double get_score( const Player player ) const;
    double evaluate( const Player player ) const;
    bool is_done( const Player player ) const;
    Move get_random_move( ) const;
    Move get_best_running_move( ) const;
    uint32_t get_hash( ) const;
    uint32_t get_lock( ) const;
    int32_t get_turns( ) const;

    class MoveIterator
    {
    public:
        explicit MoveIterator( const Board& board );

        bool valid( ) const;
        void next( );
        Move move( ) const;
        int8_t delta_moves( ) const;

    private:
        struct Data
        {
            Move move;
            uint64_t bitmask;
            int8_t count;
        };

        void try_nil_move( const Board& board );
        void push_moves( const Board& board );

        Player m_player;
        uint64_t m_bitmask;
        int32_t m_index;
        int32_t m_count;
        int8_t m_actual_moves;
        Data m_data[ MAX_MOVES ];
    };

    MoveIterator begin( ) const;
    void display( ) const;
    bool is_running( const Player player ) const;
    bool is_running( ) const;

    static Player get_teammate( const Player player );

    struct Neighbor
    {
        Neighbor( )
            : count( 0 )
            , from( 0 )
            , to( 0 )
            , check_middle( 0 )
        {
        }

        bool
        valid( ) const
        {
            return count != 0;
        }

        uint8_t
        middle( ) const
        {
            return ( from + to ) >> 1;
        }

        uint8_t count : 2;
        uint8_t from : 6;
        uint8_t to : 6;
        uint8_t check_middle : 2;
    };

    void next_player( );

    static bool has_horizontal_wall( const int32_t field );
    static bool has_vertical_wall( const int32_t field );
    static bool has_double_horizontal_wall( const int32_t field );
    static bool has_double_vertical_wall( const int32_t field );
    static bool has_no_horizontal_wall( const int32_t field );
    static bool has_no_vertical_wall( const int32_t field );

    static Neighbor* compute_neighbors_for( uint64_t bitmask );

    static void compute_estimated_moves( const Player player );

    double get_estimated_moves( const Player player ) const;

    bool player_occupies_field( const Player player, const int32_t field ) const;

    bool is_empty( const int32_t field ) const;

    void swap( const int32_t first_field, const int32_t second_field );

    bool can_do_nil_move( ) const;

    Move* get_sorted_moves( Move sorted_moves[ MAX_MOVES ] ) const;

    double evaluate_player( const Player player ) const;

private:
    Player m_player;
    Player m_teammate;
    int32_t m_player_remaining_moves;
    uint64_t m_bitmasks[ 4 ];
    uint64_t m_filled;
    int32_t m_moves[ 4 ];
    int32_t m_turns;
    bool m_solo;
};

class Board;

class Strategy
{
public:
    virtual Action get_best_action( const Board& board ) = 0;
};

class Conversion
{
public:
    static Move string_to_move( const std::string& s );
    static std::string move_to_string( const Move move );
    static Action string_to_action( const std::string& s );
    static std::string action_to_string( const Action& action );
};


class ExpectMinMaxStrategy : public Strategy
{
public:
    explicit ExpectMinMaxStrategy( const Board::Player player );
    ~ExpectMinMaxStrategy( );

    Action get_best_action( const Board& board ) override;

private:
    void init_search( );
    double search( const Board& board,
                   const double min_value,
                   const double max_value,
                   const int32_t depth,
                   Move* best_move_ptr = nullptr );

    bool look_in_transposition_table( const Board& board,
                                      const int32_t depth,
                                      double& alpha,
                                      double& beta,
                                      double& value,
                                      Move* best_move_ptr,
                                      Move& PV );

    double get_best_value( const Board& board,
                           const double alpha,
                           const double beta,
                           const int32_t depth,
                           const Move PV,
                           Move* best_move_ptr );

    double get_average_value( const Board& board,
                              const double alpha,
                              const double beta,
                              const int32_t depth );

    double get_worst_value( const Board& board,
                            const double alpha,
                            const double beta,
                            const int32_t depth,
                            const Move PV );

    Board::Player m_player;
    Board::Player m_teammate;
    int32_t m_nodes;
    int32_t m_cutoff;
    bool m_can_abort;
    bool m_aborted;
    TranpositionTable m_transposition_table;
};


class RandomStrategy : public Strategy
{
public:
    RandomStrategy( );
    ~RandomStrategy( );

    Action get_best_action( const Board& board ) override;
};


class RunStrategy : public Strategy
{
public:
    RunStrategy( );
    ~RunStrategy( );

    Action get_best_action( const Board& board ) override;
};


struct Node
{
    struct Stats
    {
        Stats( )
            : value( 0.0 )
            , visits( 0 )
        {
        }

        double value;
        int32_t visits;
    };

    struct SharedStats
    {
        Stats mc;
        std::unordered_map< Move, Stats > rave;
    };

    Node( const Board& board,
          const Move move = INVALID_MOVE,
          Node* parent = nullptr,
          const double weight = 0.0,
          const double bias = 0.0 );
    ~Node( );

    Node* expand( Board& board );
    double get_exploration_bonus( const Node* child ) const;
    void update_value( );
    Node* select_best( );
    Node* select_worst( );
    Node* select_randomly( );
    Node* select( const Board::Player reference );
    Node* select_most_visited( );
    void update( );
    void mc_update( const double new_value );
    void rave_update( const Board::Player player, const Move move, const double new_value );
    bool is_fully_expanded( ) const;

    int32_t level;
    Board::Player player;
    Move move;
    Node* parent;
    std::vector< Node* > children;
    double weight;
    double bias;
    double value;
    int32_t visits;
    std::list< Move > untried_moves;
    bool is_leaf;
    SharedStats* stats;

    static void init_data( );
    static void clear_all_nodes( );
};


class MCTSStrategy : public Strategy
{
public:
    explicit MCTSStrategy( const Board::Player player );
    ~MCTSStrategy( );

    Action get_best_action( const Board& board ) override;

private:
    struct PlayerMove
    {
        PlayerMove( Board::Player player, Move move )
            : player( player )
            , move( move )
        {
        }

        Board::Player player;
        Move move;
    };

    double run_simulation( Board& board,
                           const int32_t max_turns,
                           std::vector< PlayerMove >& moves );

    Board::Player m_player;
};


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


namespace
{
const int32_t buckets_count = 1000003;
}

TranpositionTable::TranpositionTable( )
    : m_buckets( buckets_count )
{
}

TranpositionTable::~TranpositionTable( )
{
}

const Entry*
TranpositionTable::find( const Board& board )
{
    const Entry* entry = nullptr;

    const auto board_hash = board.get_hash( );
    const auto board_lock = board.get_lock( );
    const auto bucket_index = board_hash % buckets_count;
    if ( m_buckets[ bucket_index ].lock == board_lock )
    {
        entry = &m_buckets[ bucket_index ];
    }

    return entry;
}

void
TranpositionTable::store( const Board& board,
                          const int32_t depth,
                          const Entry::Type type,
                          const double value,
                          const Move move )
{
    const auto board_hash = board.get_hash( );
    const auto board_lock = board.get_lock( );
    const auto bucket_index = board_hash % buckets_count;
    Entry* entry = &m_buckets[ bucket_index ];
    entry->lock = board_lock;
    entry->depth = depth;
    entry->type = type;
    entry->value = value;
    entry->move = move;
}


namespace
{
const int32_t MAX_TURNS = 80;

const Board::Player players[] = {Board::YELLOW, Board::BLACK, Board::WHITE, Board::RED};

const uint64_t starts[ 4 ] = {1ull << 48 | 1ull << 49 | 1ull << 56 | 1ull << 57,
                              1ull << 0 | 1ull << 1 | 1ull << 8 | 1ull << 9,
                              1ull << 54 | 1ull << 55 | 1ull << 62 | 1ull << 63,
                              1ull << 6 | 1ull << 7 | 1ull << 14 | 1ull << 15};

const uint64_t targets[ 4 ] = {starts[ 3 ], starts[ 2 ], starts[ 1 ], starts[ 0 ]};

const int32_t all_fields[ NXN ]
    = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
       22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
       44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63};

int32_t horizontal_wall_count[ NXN ];
int32_t vertical_wall_count[ NXN ];
int32_t count_moves[ NXN ][ 8 ];
int32_t move_count[ 4096 ];

constexpr int32_t MAX_BITMASKS = 635376;
constexpr int MAX_NEIGHBORS = 24 * MAX_BITMASKS;

Board::Neighbor computed_neighbors[ MAX_NEIGHBORS ];
Board::Neighbor* computed_neighbor = computed_neighbors;

const int32_t c[ 65 ][ 5 ] = {{0, 0, 0, 0, 0},
                              {1, 0, 0, 0, 0},
                              {1, 1, 0, 0, 0},
                              {1, 2, 1, 0, 0},
                              {1, 3, 3, 1, 0},
                              {1, 4, 6, 4, 1},
                              {1, 5, 10, 10, 5},
                              {1, 6, 15, 20, 15},
                              {1, 7, 21, 35, 35},
                              {1, 8, 28, 56, 70},
                              {1, 9, 36, 84, 126},
                              {1, 10, 45, 120, 210},
                              {1, 11, 55, 165, 330},
                              {1, 12, 66, 220, 495},
                              {1, 13, 78, 286, 715},
                              {1, 14, 91, 364, 1001},
                              {1, 15, 105, 455, 1365},
                              {1, 16, 120, 560, 1820},
                              {1, 17, 136, 680, 2380},
                              {1, 18, 153, 816, 3060},
                              {1, 19, 171, 969, 3876},
                              {1, 20, 190, 1140, 4845},
                              {1, 21, 210, 1330, 5985},
                              {1, 22, 231, 1540, 7315},
                              {1, 23, 253, 1771, 8855},
                              {1, 24, 276, 2024, 10626},
                              {1, 25, 300, 2300, 12650},
                              {1, 26, 325, 2600, 14950},
                              {1, 27, 351, 2925, 17550},
                              {1, 28, 378, 3276, 20475},
                              {1, 29, 406, 3654, 23751},
                              {1, 30, 435, 4060, 27405},
                              {1, 31, 465, 4495, 31465},
                              {1, 32, 496, 4960, 35960},
                              {1, 33, 528, 5456, 40920},
                              {1, 34, 561, 5984, 46376},
                              {1, 35, 595, 6545, 52360},
                              {1, 36, 630, 7140, 58905},
                              {1, 37, 666, 7770, 66045},
                              {1, 38, 703, 8436, 73815},
                              {1, 39, 741, 9139, 82251},
                              {1, 40, 780, 9880, 91390},
                              {1, 41, 820, 10660, 101270},
                              {1, 42, 861, 11480, 111930},
                              {1, 43, 903, 12341, 123410},
                              {1, 44, 946, 13244, 135751},
                              {1, 45, 990, 14190, 148995},
                              {1, 46, 1035, 15180, 163185},
                              {1, 47, 1081, 16215, 178365},
                              {1, 48, 1128, 17296, 194580},
                              {1, 49, 1176, 18424, 211876},
                              {1, 50, 1225, 19600, 230300},
                              {1, 51, 1275, 20825, 249900},
                              {1, 52, 1326, 22100, 270725},
                              {1, 53, 1378, 23426, 292825},
                              {1, 54, 1431, 24804, 316251},
                              {1, 55, 1485, 26235, 341055},
                              {1, 56, 1540, 27720, 367290},
                              {1, 57, 1596, 29260, 395010},
                              {1, 58, 1653, 30856, 424270},
                              {1, 59, 1711, 32509, 455126},
                              {1, 60, 1770, 34220, 487635},
                              {1, 61, 1830, 35990, 521855},
                              {1, 62, 1891, 37820, 557845},
                              {1, 63, 1953, 39711, 595665}};

struct Store
{
public:
    struct Node
    {
        Node( )
            : moves{-1, -1, -1, -1}
            , neighbor( nullptr )
        {
        }

        int8_t moves[ 4 ];
        Board::Neighbor* neighbor;
    };

    Node& operator[]( uint64_t bitmask )
    {
        uint64_t b = bitmask;
        int32_t index = 0;
        int32_t d = 1;
        int32_t last = 0;
        for ( int32_t l = 0; l < 4; ++l )
        {
            int32_t a = __builtin_ffsll( b );
            index += c[ last + a ][ d++ ];
            b >>= a;
            last += a;
        }

        return nodes[ index ];
    }

    Node nodes[ MAX_BITMASKS ];
};

Store stored;

struct ZobristData
{
    uint32_t init;
    uint32_t player[ 4 ];
    uint32_t remaining_moves[ 4 ];
    uint32_t moves[ 4 ][ 100 ];
    uint32_t fields[ 4 ][ NXN ];
} hash_data, lock_data;

const Board::Player first_team[ 2 ] = {Board::YELLOW, Board::WHITE};
const Board::Player second_team[ 2 ] = {Board::BLACK, Board::RED};

int32_t
get_move_count( const Move move )
{
    const int32_t from = move & 0x3f;
    const int32_t to = ( move >> 6 ) & 0x3f;
    if ( to == from + 1 )
    {
        return count_moves[ from ][ Board::RIGHT_1 ];
    }
    else if ( from == to + 1 )
    {
        return count_moves[ from ][ Board::LEFT_1 ];
    }
    else if ( to == from + N )
    {
        return count_moves[ from ][ Board::DOWN_1 ];
    }
    else if ( from == to + N )
    {
        return count_moves[ from ][ Board::UP_1 ];
    }
    else
    {
        return 1;
    }
}

const std::vector< double > weights = []( ) {
    const int32_t weights_count = 8;
    const double weight_base = 16.0;
    std::vector< double > weights( weights_count );
    for ( auto i = 0; i < weights_count; ++i )
    {
        weights[ i ] = std::pow( weight_base, i - 6 );
    }

    return weights;
}( );

uint64_t
get_neighbor_bitmask( uint64_t bitmask, Board::Neighbor* neighbor )
{
    return bitmask ^ ( 1ull << neighbor->from ) ^ ( 1ull << neighbor->to );
}
}

Board::Neighbor*
Board::compute_neighbors_for( uint64_t bitmask )
{
    Board::Neighbor* result = computed_neighbor;
    Board::Neighbor* neighbor = computed_neighbor;

    auto empty = [bitmask]( int32_t f ) { return ( bitmask & ( 1ull << f ) ) == 0; };

    for ( int32_t field : all_fields )
    {
        if ( ( bitmask & ( 1ull << field ) ) == 0 )
        {
            continue;
        }

        if ( count_moves[ field ][ RIGHT_1 ] != OO and empty( field + 1 ) )
        {
            neighbor->count = count_moves[ field ][ RIGHT_1 ];
            neighbor->from = field;
            neighbor->to = field + 1;
            neighbor->check_middle = 0;

            ++neighbor;
        }

        if ( count_moves[ field ][ RIGHT_2 ] != OO and empty( field + 2 ) )
        {
            neighbor->count = count_moves[ field ][ RIGHT_2 ];
            neighbor->from = field;
            neighbor->to = field + 2;
            neighbor->check_middle = empty( field + 1 );

            ++neighbor;
        }

        if ( count_moves[ field ][ LEFT_1 ] != OO and empty( field - 1 ) )
        {
            neighbor->count = count_moves[ field ][ LEFT_1 ];
            neighbor->from = field;
            neighbor->to = field - 1;
            neighbor->check_middle = 0;

            ++neighbor;
        }

        if ( count_moves[ field ][ LEFT_2 ] != OO and empty( field - 2 ) )
        {
            neighbor->count = count_moves[ field ][ LEFT_2 ];
            neighbor->from = field;
            neighbor->to = field - 2;
            neighbor->check_middle = empty( field - 1 );

            ++neighbor;
        }

        if ( count_moves[ field ][ UP_1 ] != OO and empty( field - N ) )
        {
            neighbor->count = count_moves[ field ][ UP_1 ];
            neighbor->from = field;
            neighbor->to = field - N;
            neighbor->check_middle = 0;

            ++neighbor;
        }

        if ( count_moves[ field ][ UP_2 ] != OO and empty( field - 2 * N ) )
        {
            neighbor->count = count_moves[ field ][ UP_2 ];
            neighbor->from = field;
            neighbor->to = field - 2 * N;
            neighbor->check_middle = empty( field - N );

            ++neighbor;
        }

        if ( count_moves[ field ][ DOWN_1 ] != OO and empty( field + N ) )
        {
            neighbor->count = count_moves[ field ][ DOWN_1 ];
            neighbor->from = field;
            neighbor->to = field + N;
            neighbor->check_middle = 0;

            ++neighbor;
        }

        if ( count_moves[ field ][ DOWN_2 ] != OO and empty( field + 2 * N ) )
        {
            neighbor->count = count_moves[ field ][ DOWN_2 ];
            neighbor->from = field;
            neighbor->to = field + 2 * N;
            neighbor->check_middle = empty( field + N );

            ++neighbor;
        }
    }

    computed_neighbor = ++neighbor;

    return result;
}

Board::Board( )
    : m_player( YELLOW )
    , m_teammate( WHITE )
    , m_player_remaining_moves( 3 )
    , m_bitmasks{starts[ 0 ], starts[ 1 ], starts[ 2 ], starts[ 3 ]}
    , m_filled( starts[ 0 ] | starts[ 1 ] | starts[ 2 ] | starts[ 3 ] )
    , m_moves{0, 0, 0, 0}
    , m_turns( 0 )
    , m_solo( false )
{
}

Board::Board( const Player player, const uint64_t bitmasks[ 4 ] )
    : m_player( player )
    , m_teammate( get_teammate( player ) )
    , m_player_remaining_moves( 3 )
    , m_bitmasks{bitmasks[ 0 ], bitmasks[ 1 ], bitmasks[ 2 ], bitmasks[ 3 ]}
    , m_filled( bitmasks[ 0 ] | bitmasks[ 1 ] | bitmasks[ 2 ] | bitmasks[ 3 ] )
    , m_moves{0, 0, 0, 0}
    , m_turns( 0 )
    , m_solo( false )
{
}

Board::Board( const Player player, const uint64_t bitmask )
    : m_player( player )
    , m_teammate( get_teammate( player ) )
    , m_player_remaining_moves( 3 )
    , m_bitmasks{0ull, 0ull, 0ull, 0ull}
    , m_moves{0, 0, 0, 0}
    , m_turns( 0 )
    , m_solo( true )
{
    m_bitmasks[ m_player ] = bitmask;
    m_filled = bitmask;
}

void
Board::init_data( const std::string& walls )
{
    Timer timer;

    int32_t index = 0;

    for ( int32_t i = 0; i < N; ++i )
    {
        for ( int32_t j = 0; j < N - 1; ++j )
        {
            const int32_t count = walls[ index++ ] - '0';
            const int32_t field = N * i + j;
            vertical_wall_count[ field ] = count;
        }

        if ( i < N - 1 )
        {
            for ( int32_t j = 0; j < N; ++j )
            {
                const int32_t count = walls[ index++ ] - '0';
                const int32_t field = N * i + j;
                horizontal_wall_count[ field ] = count;
            }
        }
    }

    for ( int32_t field : all_fields )
    {
        // right_1
        if ( field % N != 7 )
        {
            count_moves[ field ][ RIGHT_1 ] = 1 + vertical_wall_count[ field ];
        }
        else
        {
            count_moves[ field ][ RIGHT_1 ] = OO;
        }
        // right_2
        if ( field % N < 6 and vertical_wall_count[ field ] == 0
             and vertical_wall_count[ field + 1 ] == 0 )
        {
            count_moves[ field ][ RIGHT_2 ] = 1;
        }
        else
        {
            count_moves[ field ][ RIGHT_2 ] = OO;
        }
        // left_1
        if ( field % N != 0 )
        {
            count_moves[ field ][ LEFT_1 ] = 1 + vertical_wall_count[ field - 1 ];
        }
        else
        {
            count_moves[ field ][ LEFT_1 ] = OO;
        }
        // left_2
        if ( field % N > 1 and vertical_wall_count[ field - 2 ] == 0
             and vertical_wall_count[ field - 1 ] == 0 )
        {
            count_moves[ field ][ LEFT_2 ] = 1;
        }
        else
        {
            count_moves[ field ][ LEFT_2 ] = OO;
        }
        // up_1
        if ( field >= N )
        {
            count_moves[ field ][ UP_1 ] = 1 + horizontal_wall_count[ field - N ];
        }
        else
        {
            count_moves[ field ][ UP_1 ] = OO;
        }
        // up_2
        if ( field >= 2 * N and horizontal_wall_count[ field - 2 * N ] == 0
             and horizontal_wall_count[ field - N ] == 0 )
        {
            count_moves[ field ][ UP_2 ] = 1;
        }
        else
        {
            count_moves[ field ][ UP_2 ] = OO;
        }
        // down_1
        if ( field + N < NXN )
        {
            count_moves[ field ][ DOWN_1 ] = 1 + horizontal_wall_count[ field ];
        }
        else
        {
            count_moves[ field ][ DOWN_1 ] = OO;
        }
        // down_2
        if ( field + 2 * N < NXN and horizontal_wall_count[ field ] == 0
             and horizontal_wall_count[ field + N ] == 0 )
        {
            count_moves[ field ][ DOWN_2 ] = 1;
        }
        else
        {
            count_moves[ field ][ DOWN_2 ] = OO;
        }
    }

    for ( ZobristData* zobrist : {&hash_data, &lock_data} )
    {
        zobrist->init = RandomNumberGenerator::pick( );

        for ( auto i = 0; i < 4; ++i )
        {
            zobrist->remaining_moves[ i ] = RandomNumberGenerator::pick( );
        }

        for ( auto p : players )
        {
            zobrist->player[ p ] = RandomNumberGenerator::pick( );

            for ( int32_t f : all_fields )
            {
                zobrist->fields[ p ][ f ] = RandomNumberGenerator::pick( );
            }

            for ( int32_t m = 0; m < 100; ++m )
            {
                zobrist->moves[ p ][ m ] = RandomNumberGenerator::pick( );
            }
        }
    }

    for ( int32_t m = 0; m < 4096; ++m )
    {
        move_count[ m ] = get_move_count( m );
    }

    timer.stop( );
    std::cerr << "Data init tooks " << timer.get_delta_time( ) << " sec\n";
}

void
Board::pre_compute( )
{
    Timer timer;

    for ( int a = 0; a < 64; ++a )
    {
        for ( int b = a + 1; b < 64; ++b )
        {
            for ( int c = b + 1; c < 64; ++c )
            {
                for ( int d = c + 1; d < 64; ++d )
                {
                    uint64_t bitmask = 1ull << a | 1ull << b | 1ull << c | 1ull << d;
                    stored[ bitmask ].neighbor = compute_neighbors_for( bitmask );
                }
            }
        }
    }

    timer.stop( );
    const double neighbors_delta_time = timer.get_delta_time( );
    std::cerr << "Computing neighbors time = " << neighbors_delta_time << " sec\n";
    std::cerr << "Neighbors usage = "
              << 100 * ( computed_neighbor - computed_neighbors ) / ( 24 * MAX_BITMASKS ) << "%\n";
    std::cerr << "Neighbors memory = " << sizeof( computed_neighbors ) / 1e6 << "M\n";

    timer.reset( );

    for ( const auto player : players )
    {
        compute_estimated_moves( player );
    }

    timer.stop( );
    std::cerr << "Computing estimated moves time = " << timer.get_delta_time( ) << " sec\n";
    std::cerr << "Estimated moves memory = " << sizeof( Store ) / 1e6 << "M\n";
    std::cerr << "Total initialization time = " << timer.get_total_time( ) << " sec\n";
}

Board::~Board( )
{
}

Board::Player
Board::get_player( ) const
{
    return m_player;
}

int32_t
Board::do_move( const Move move )
{
    int32_t count = 0;

    if ( move == NIL_MOVE )
    {
        next_player( );
    }
    else
    {
        const int32_t from = move & 0x3f;
        const int32_t to = ( move >> 6 ) & 0x3f;

        swap( from, to );
        count = move_count[ move ];
        m_moves[ m_player ] += count;

        if ( not m_solo )
        {
            m_player_remaining_moves -= count;
            if ( m_player_remaining_moves == 0 )
            {
                next_player( );
            }
        }
    }

    return count;
}

void
Board::do_action( const Action& action )
{
    const auto player = m_player;
    for ( const auto move : action )
    {
        do_move( move );
    }

    //! Check if the next player is called
    if ( m_player == player )
    {
        next_player( );
    }
}

void
Board::enable_solo_mode( const Player player )
{
    for ( const auto p : players )
    {
        if ( p != player )
        {
            m_bitmasks[ p ] = 0ull;
        }
    }

    m_filled = m_bitmasks[ player ];

    m_player = player;
    m_teammate = get_teammate( player );
    m_player_remaining_moves = 3;

    m_solo = true;
}

bool
Board::end_game( ) const
{
    return ( m_turns == MAX_TURNS )
           or ( is_done( YELLOW ) and is_done( WHITE ) and is_done( BLACK ) and is_done( RED ) );
}

double
Board::get_mobility( const Player ) const
{
    return 0.0;
}

double
Board::get_estimated_moves( const Player player ) const
{
    return m_moves[ player ] + stored[ m_bitmasks[ player ] ].moves[ player ];
}

double
Board::get_score( const Player player ) const
{
    double score = 0.0;

    for ( const auto p : first_team )
    {
        score -= get_estimated_moves( p );
    }

    for ( const auto p : second_team )
    {
        score += get_estimated_moves( p );
    }

    if ( player == BLACK or player == RED )
    {
        score = -score;
    }

    return score;
}

double
Board::evaluate( const Player player ) const
{
    double eval = 0.0;

    for ( const auto p : first_team )
    {
        eval += evaluate_player( p );
    }

    for ( const auto p : second_team )
    {
        eval -= evaluate_player( p );
    }

    return player == YELLOW or player == WHITE ? eval : -eval;
}

Board::MoveIterator::MoveIterator( const Board& board )
    : m_player( board.m_player )
    , m_bitmask( board.m_bitmasks[ m_player ] )
    , m_index( 0 )
    , m_count( 0 )
{
    if ( not board.is_done( board.m_player ) )
    {
        push_moves( board );
    }
    else if ( not board.is_done( board.m_teammate ) )
    {
        m_player = board.m_teammate;
        m_bitmask = board.m_bitmasks[ m_player ];
        push_moves( board );
    }

    try_nil_move( board );

    m_actual_moves = stored[ m_bitmask ].moves[ m_player ];
}

void
Board::MoveIterator::try_nil_move( const Board& board )
{
    if ( board.can_do_nil_move( ) )
    {
        auto& data = m_data[ m_count ];
        data.move = NIL_MOVE;
        data.bitmask = m_bitmask;
        data.count = board.m_player_remaining_moves;

        ++m_count;
    }
}

void
Board::MoveIterator::push_moves( const Board& board )
{
    for ( auto neighbor = stored[ m_bitmask ].neighbor; neighbor->valid( ); ++neighbor )
    {
        if ( neighbor->count > board.m_player_remaining_moves )
        {
            continue;
        }

        if ( not board.is_empty( neighbor->to ) )
        {
            continue;
        }

        if ( neighbor->check_middle and board.is_empty( neighbor->middle( ) ) )
        {
            continue;
        }

        auto& data = m_data[ m_count ];

        data.move = CREATE_MOVE( static_cast< int32_t >( neighbor->from ),
                                 static_cast< int32_t >( neighbor->to ) );

        data.bitmask = get_neighbor_bitmask( m_bitmask, neighbor );
        data.count = neighbor->count;

        m_count++;
    }
}

Move
Board::MoveIterator::move( ) const
{
    return m_data[ m_index ].move;
}

void
Board::MoveIterator::next( )
{
    if ( m_index < m_count )
    {
        ++m_index;
    }
}

int8_t
Board::MoveIterator::delta_moves( ) const
{
    auto& data = m_data[ m_index ];
    return m_actual_moves - stored[ data.bitmask ].moves[ m_player ] - data.count;
}

bool
Board::MoveIterator::valid( ) const
{
    return m_index < m_count;
}

Board::MoveIterator
Board::begin( ) const
{
    return MoveIterator( *this );
}

void
Board::display( ) const
{
    std::cerr << "\n    ";
    for ( int32_t i = 1; i <= N; ++i )
    {
        std::cerr << "   " << i << "   ";
    }
    std::cerr << "\n" << std::string( 60, '-' ) << "\n";

    for ( int32_t i = 0; i < N; ++i )
    {
        for ( int32_t j = 0; j < N; ++j )
        {
            if ( j == 0 )
            {
                std::cerr << ( (char)( 'a' + i ) ) << "|    ";
            }
            const int32_t f = N * i + j;
            const uint64_t flag = 1ull << f;
            bool empty = true;
            for ( const auto player : players )
            {
                if ( m_bitmasks[ player ] & flag )
                {
                    std::cerr << " " << player << " ";
                    empty = false;
                }
            }

            if ( empty )
            {
                std::cerr << " . ";
            }

            if ( has_no_vertical_wall( f ) )
            {
                std::cerr << "    ";
            }
            else if ( has_double_vertical_wall( f ) )
            {
                std::cerr << " || ";
            }
            else
            {
                std::cerr << "  | ";
            }

            if ( j == N - 1 )
            {
                std::cerr << "\n";
            }
        }

        if ( i < N - 1 )
        {
            for ( int32_t j = 0; j < N; ++j )
            {
                if ( j == 0 )
                {
                    std::cerr << " |  ";
                }

                const int32_t f = N * i + j;

                if ( has_no_horizontal_wall( f ) )
                {
                    std::cerr << "       ";
                }
                else if ( has_double_horizontal_wall( f ) )
                {
                    std::cerr << "  ===  ";
                }
                else
                {
                    std::cerr << "  ---  ";
                }

                if ( j == N - 1 )
                {
                    std::cerr << "\n";
                }
            }
        }
    }
    std::cerr << "\nplayer=" << m_player << "\n";
    for ( int32_t i = 0; i < 4; ++i )
    {
        std::cerr << "moves_" << i << "=" << m_moves[ i ] << "\n";
    }
    std::cerr << "remainig moves=" << m_player_remaining_moves << "\n";
    std::cerr << "turns=" << m_turns << "\n";
    std::cerr << "bitmasks={";
    for ( int32_t i = 0; i < 4; ++i )
    {
        if ( i != 0 )
            std::cerr << ", ";
        std::cerr << m_bitmasks[ i ] << "ull";
    }
    std::cerr << "}\n";
    std::cerr << "\n";
}

void
Board::next_player( )
{
    if ( not is_done( m_player ) or not is_done( m_teammate ) )
    {
        m_moves[ m_player ] += m_player_remaining_moves;
    }

    switch ( m_player )
    {
    case YELLOW:
        m_player = BLACK;
        m_teammate = RED;
        break;
    case BLACK:
        m_player = WHITE;
        m_teammate = YELLOW;
        break;
    case WHITE:
        m_player = RED;
        m_teammate = BLACK;
        break;
    case RED:
        m_player = YELLOW;
        m_teammate = WHITE;
        break;
    }

    ++m_turns;
    m_player_remaining_moves = 3;
}

Board::Player
Board::get_teammate( const Player player )
{
    static const Player teammate[] = {WHITE, RED, YELLOW, BLACK};

    return teammate[ player ];
}

bool
Board::has_horizontal_wall( const int32_t field )
{
    return horizontal_wall_count[ field ] != 0;
}

bool
Board::has_vertical_wall( const int32_t field )
{
    return vertical_wall_count[ field ] != 0;
}

bool
Board::has_double_horizontal_wall( const int32_t field )
{
    return horizontal_wall_count[ field ] == 2;
}

bool
Board::has_double_vertical_wall( const int32_t field )
{
    return vertical_wall_count[ field ] == 2;
}

bool
Board::has_no_horizontal_wall( const int32_t field )
{
    return horizontal_wall_count[ field ] == 0;
}

bool
Board::has_no_vertical_wall( const int32_t field )
{
    return vertical_wall_count[ field ] == 0;
}

bool
Board::player_occupies_field( const Board::Player player, const int32_t field ) const
{
    const uint64_t flag = 1ull << field;
    return ( m_bitmasks[ player ] & flag ) != 0ull;
}

bool
Board::is_done( const Player player ) const
{
    return m_bitmasks[ player ] == targets[ player ];
}

Move
Board::get_random_move( ) const
{
    using WeightMove = std::pair< double, Move >;

    WeightMove weighted_moves[ MAX_MOVES ];

    double sum_weights = 0.0;
    WeightMove* weighted_moves_end = weighted_moves;
    for ( auto iterator = begin( ); iterator.valid( ); iterator.next( ) )
    {
        const auto move = iterator.move( );
        const int32_t index = iterator.delta_moves( ) + 6;
        sum_weights += weights[ index ];
        weighted_moves_end->first = sum_weights;
        weighted_moves_end->second = move;
        ++weighted_moves_end;
    }

    if ( weighted_moves_end == weighted_moves )
    {
        return INVALID_MOVE;
    }

    const auto random_weight = RandomNumberGenerator::pick( sum_weights );
    const WeightMove* iterator = std::upper_bound( weighted_moves, weighted_moves_end,
                                                   WeightMove{random_weight, INVALID_MOVE} );

    return iterator->second;
}

bool
Board::is_empty( const int32_t field ) const
{
    const uint64_t flag = 1ull << field;
    return ( m_filled & flag ) == 0ull;
}

void
Board::swap( const int32_t first_field, const int32_t second_field )
{
    const auto player = is_done( m_player ) ? m_teammate : m_player;
    const uint64_t first_flag = 1ull << first_field;
    const uint64_t second_flag = 1ull << second_field;
    m_bitmasks[ player ] ^= first_flag | second_flag;
    m_filled ^= first_flag | second_flag;
}

bool
Board::can_do_nil_move( ) const
{
    return ( m_player_remaining_moves < 3 )
           or ( m_bitmasks[ m_player ] == targets[ m_player ]
                and m_bitmasks[ m_teammate ] == targets[ m_teammate ] );
}

uint32_t
Board::get_hash( ) const
{
    ZobristData* zobrist = &hash_data;

    uint32_t h = zobrist->init;

    h ^= zobrist->player[ m_player ];
    h ^= zobrist->remaining_moves[ m_player_remaining_moves ];
    h ^= zobrist->moves[ m_player ][ m_moves[ m_player ] ];

    for ( Player p : players )
    {
        h ^= zobrist->moves[ p ][ m_moves[ p ] ];
    }

    for ( int32_t field : all_fields )
    {
        const uint64_t flag = 1ull << field;
        for ( const auto player : players )
        {
            if ( m_bitmasks[ player ] & flag )
            {
                h ^= zobrist->fields[ player ][ field ];
            }
        }
    }

    return h;
}

uint32_t
Board::get_lock( ) const
{
    ZobristData* zobrist = &lock_data;

    uint32_t l = zobrist->init;

    l ^= zobrist->player[ m_player ];
    l ^= zobrist->remaining_moves[ m_player_remaining_moves ];
    l ^= zobrist->moves[ m_player ][ m_moves[ m_player ] ];

    for ( Player p : players )
    {
        l ^= zobrist->moves[ p ][ m_moves[ p ] ];
    }

    for ( int32_t field : all_fields )
    {
        const uint64_t flag = 1ull << field;
        for ( const auto player : players )
        {
            if ( m_bitmasks[ player ] & flag )
            {
                l ^= zobrist->fields[ player ][ field ];
            }
        }
    }

    return l;
}

int32_t
Board::get_turns( ) const
{
    return m_turns;
}

void
Board::compute_estimated_moves( const Player player )
{
    std::queue< uint64_t > q;
    const uint64_t target = targets[ player ];

    stored[ target ].moves[ player ] = 0;
    q.push( target );

    while ( not q.empty( ) )
    {
        auto top = q.front( );
        q.pop( );

        auto& node = stored[ top ];

        int8_t& d_top = node.moves[ player ];

        for ( auto neighbor = node.neighbor; neighbor->valid( ); ++neighbor )
        {
            if ( neighbor->check_middle )
            {
                continue;
            }

            const int32_t proposed_count = d_top + neighbor->count;
            const uint64_t neighbor_bitmask = get_neighbor_bitmask( top, neighbor );
            int8_t& d_neighbor_bitmask = stored[ neighbor_bitmask ].moves[ player ];

            if ( d_neighbor_bitmask == -1 or d_neighbor_bitmask > proposed_count )
            {
                d_neighbor_bitmask = proposed_count;
                q.push( neighbor_bitmask );
            }
        }
    }
}

bool
Board::is_running( const Player p ) const
{
    if ( m_turns < 24 )
    {
        return false;
    }

    auto player = is_done( p ) ? get_teammate( p ) : p;

    int32_t corner = 0;
    switch ( player )
    {
    case YELLOW:
        corner = 7;
        break;
    case BLACK:
        corner = 63;
        break;
    case WHITE:
        corner = 0;
        break;
    case RED:
        corner = 56;
        break;
    }

    int32_t min_x = corner / N;
    int32_t max_x = corner / N;
    int32_t min_y = corner % N;
    int32_t max_y = corner % N;

    for ( int32_t f : all_fields )
    {
        if ( not player_occupies_field( player, f ) )
        {
            continue;
        }

        int32_t x = f / N;
        int32_t y = f % N;

        min_x = std::min( min_x, x );
        max_x = std::max( max_x, x );
        min_y = std::min( min_y, y );
        max_y = std::max( max_y, y );
    }

    for ( int32_t x = min_x; x <= max_x; ++x )
    {
        for ( int32_t y = min_y; y <= max_y; ++y )
        {
            int32_t f = N * x + y;
            if ( not is_empty( f ) and not player_occupies_field( player, f ) )
            {
                return false;
            }
        }
    }

    return true;
}

bool
Board::is_running( ) const
{
    return is_running( YELLOW ) and is_running( BLACK ) and is_running( WHITE )
           and is_running( RED );
}

Move
Board::get_best_running_move( ) const
{
    int32_t max_delta_moves = std::numeric_limits< int32_t >::lowest( );
    Move running_move = INVALID_MOVE;

    for ( auto iterator = begin( ); iterator.valid( ); iterator.next( ) )
    {
        const auto move = iterator.move( );
        const int32_t delta_moves = iterator.delta_moves( );
        if ( max_delta_moves < delta_moves )
        {
            max_delta_moves = delta_moves;
            running_move = move;
        }
    }

    return running_move;
}

Move*
Board::get_sorted_moves( Move sorted_moves[ MAX_MOVES ] ) const
{
    Move moves[ 8 ][ 16 ];
    Move* moves_end[ 8 ] = {moves[ 0 ], moves[ 1 ], moves[ 2 ], moves[ 3 ],
                            moves[ 4 ], moves[ 5 ], moves[ 6 ], moves[ 7 ]};

    for ( auto iterator = begin( ); iterator.valid( ); iterator.next( ) )
    {
        const auto move = iterator.move( );
        const int32_t index = iterator.delta_moves( ) + 6;
        *moves_end[ index ]++ = move;
    }

    Move* sorted_moves_iterator = sorted_moves;

    for ( int32_t i = 7; i >= 0; --i )
    {
        sorted_moves_iterator = std::copy( moves[ i ], moves_end[ i ], sorted_moves_iterator );
    }

    return sorted_moves_iterator;
}

double
Board::evaluate_player( const Player player ) const
{
    const double estimated_moves_weight = -1.0;
    const double mobility_weight = 0.06;

    const double estimated_moves = get_estimated_moves( player );
    const uint64_t bitmask = m_bitmasks[ player ];
    const auto& data = stored[ bitmask ];
    const int8_t moves = data.moves[ player ];

    double mobility = 0.0;
    for ( Neighbor* neighbor = data.neighbor; neighbor->valid( ); ++neighbor )
    {
        if ( not is_empty( neighbor->to ) )
        {
            continue;
        }

        if ( neighbor->check_middle and is_empty( neighbor->middle( ) ) )
        {
            continue;
        }

        const uint64_t neighbor_bitmask = get_neighbor_bitmask( bitmask, neighbor );
        const int64_t neighbor_moves = stored[ neighbor_bitmask ].moves[ player ];

        if ( neighbor_moves < moves )
        {
            mobility += moves - neighbor_moves;
        }
    }

    return estimated_moves_weight * estimated_moves + mobility_weight * mobility;
}



namespace
{
bool
is_valid( const Board& board, const Move PV )
{
    return PV != INVALID_MOVE;
}

Entry::Type
get_entry_type( const double value, const double alpha, const double beta )
{
    if ( value <= alpha )
    {
        return Entry::UPPER;
    }

    if ( value >= beta )
    {
        return Entry::LOWER;
    }

    return Entry::EXACT;
}

int32_t
get_next_depth( const Board& board, const Board& next_board, const int32_t depth )
{
    return board.get_player( ) == next_board.get_player( ) ? depth : depth - 1;
}

const int32_t START_DEPTH = 1;
const int32_t MIN_DEPTH = 2;
const int32_t MAX_DEPTH = 2;
const int32_t MAX_NODES = 2000000;

const double DEFAULT_BRANCHING_FACTOR = 10.0;
}

ExpectMinMaxStrategy::ExpectMinMaxStrategy( const Board::Player player )
    : m_player( player )
    , m_teammate( Board::get_teammate( player ) )
    , m_nodes( 0 )
    , m_cutoff( 0 )
    , m_can_abort( false )
    , m_aborted( false )
{
}

ExpectMinMaxStrategy::~ExpectMinMaxStrategy( )
{
}

void
ExpectMinMaxStrategy::init_search( )
{
    m_nodes = 0;
    m_cutoff = 0;
    m_aborted = false;
}

bool
ExpectMinMaxStrategy::look_in_transposition_table( const Board& board,
                                                   const int32_t depth,
                                                   double& alpha,
                                                   double& beta,
                                                   double& value,
                                                   Move* best_move_ptr,
                                                   Move& PV )
{
    const auto entry = m_transposition_table.find( board );
    if ( entry != nullptr and entry->depth == depth )
    {
        switch ( entry->type )
        {
        case Entry::EXACT:
            ++m_nodes;
            if ( best_move_ptr != nullptr )
            {
                *best_move_ptr = entry->move;
            }
            value = entry->value;
            return true;
        case Entry::LOWER:
            if ( entry->value >= beta )
            {
                ++m_nodes;
                value = entry->value;
                return true;
            }
            else
            {
                alpha = std::max( alpha, entry->value );
            }
            break;
        case Entry::UPPER:
            if ( entry->value <= alpha )
            {
                ++m_nodes;
                value = entry->value;
                return true;
            }
            else
            {
                beta = std::min( beta, entry->value );
            }
            break;
        }
    }

    PV = entry != nullptr ? entry->move : INVALID_MOVE;

    return false;
}

double
ExpectMinMaxStrategy::get_best_value( const Board& board,
                                      const double alpha,
                                      const double beta,
                                      const int32_t depth,
                                      const Move PV,
                                      Move* best_move_ptr )
{
    Move best_move = INVALID_MOVE;
    double best_value = -OO;
    double local_alpha = alpha;
    bool zero_move = true;
    auto process_move = [&]( const Move move ) {
        zero_move = false;
        Board next_board = board;
        next_board.do_move( move );
        const int32_t next_depth = get_next_depth( board, next_board, depth );
        double value = search( next_board, local_alpha, beta, next_depth );
        if ( best_value < value )
        {
            best_value = value;
            best_move = move;
        }

        local_alpha = std::max( local_alpha, value );
    };

    if ( is_valid( board, PV ) )
    {
        process_move( PV );
    }

    Move sorted_moves[ MAX_MOVES ];
    auto sorted_moves_end = board.get_sorted_moves( sorted_moves );
    for ( auto move_ptr = sorted_moves; move_ptr != sorted_moves_end; ++move_ptr )
    {
        if ( best_value >= beta )
        {
            ++m_cutoff;
            break;
        }

        if ( *move_ptr != PV )
        {
            process_move( *move_ptr );
            if ( m_aborted )
            {
                return 0.0;
            }
        }
    }

    if ( best_move_ptr != nullptr )
    {
        *best_move_ptr = best_move;
    }

    if ( zero_move )
    {
        return board.get_score( m_player );
    }

    auto entry_type = get_entry_type( best_value, alpha, beta );
    m_transposition_table.store( board, depth, entry_type, best_value, best_move );

    return best_value;
}

double
ExpectMinMaxStrategy::get_average_value( const Board& board,
                                         const double alpha,
                                         const double beta,
                                         const int32_t depth )
{
    double sum_values = 0.0;
    double sum_weights = 0.0;
    bool zero_move = true;
    for ( auto iterator = board.begin( ); iterator.valid( ); iterator.next( ) )
    {
        zero_move = false;
        Board next_board = board;
        next_board.do_move( iterator.move( ) );
        const double weight = std::pow( 10.0, iterator.delta_moves( ) );
        const int32_t next_depth = get_next_depth( board, next_board, depth );
        const double value = search( next_board, alpha, beta, next_depth );
        sum_values += weight * value;
        sum_weights += weight;

        if ( m_aborted )
        {
            return 0.0;
        }
    }

    if ( zero_move )
    {
        return board.get_score( m_player );
    }

    const double average_value = sum_values / sum_weights;
    auto entry_type = get_entry_type( average_value, alpha, beta );
    m_transposition_table.store( board, depth, entry_type, average_value, INVALID_MOVE );

    return average_value;
}

double
ExpectMinMaxStrategy::get_worst_value( const Board& board,
                                       const double alpha,
                                       const double beta,
                                       const int32_t depth,
                                       const Move PV )
{
    double worst_value = OO;
    Move worst_move = INVALID_MOVE;
    double local_beta = beta;
    bool zero_move = true;
    auto process_move = [&]( const Move move ) {
        zero_move = false;
        Board next_board = board;
        next_board.do_move( move );
        const int32_t next_depth = get_next_depth( board, next_board, depth );
        double value = search( next_board, alpha, local_beta, next_depth );
        if ( worst_value > value )
        {
            worst_value = value;
            worst_move = move;
        }
        local_beta = std::min( local_beta, value );
    };

    if ( is_valid( board, PV ) )
    {
        process_move( PV );
    }

    Move sorted_moves[ MAX_MOVES ];
    auto sorted_moves_end = board.get_sorted_moves( sorted_moves );
    for ( auto move_ptr = sorted_moves; move_ptr != sorted_moves_end; ++move_ptr )
    {
        if ( worst_value <= alpha )
        {
            ++m_cutoff;
            break;
        }

        if ( *move_ptr != PV )
        {
            process_move( *move_ptr );
            if ( m_aborted )
            {
                return 0.0;
            }
        }
    }

    if ( zero_move )
    {
        return board.get_score( m_player );
    }

    auto entry_type = get_entry_type( worst_value, alpha, beta );
    m_transposition_table.store( board, depth, entry_type, worst_value, worst_move );

    return worst_value;
}

double
ExpectMinMaxStrategy::search( const Board& board,
                              const double min_value,
                              const double max_value,
                              const int32_t depth,
                              Move* best_move_ptr )
{
    if ( m_can_abort and m_nodes >= MAX_NODES )
    {
        m_aborted = true;
        return 0.0;
    }

    if ( board.end_game( ) )
    {
        ++m_nodes;
        if ( best_move_ptr != nullptr )
        {
            *best_move_ptr = NIL_MOVE;
        }

        return board.get_score( m_player );
    }

    if ( depth == 0 )
    {
        ++m_nodes;
        if ( best_move_ptr != nullptr )
        {
            *best_move_ptr = NIL_MOVE;
        }

        return board.evaluate( m_player );
    }

    double alpha = min_value;
    double beta = max_value;
    double value;
    Move PV;

    if ( look_in_transposition_table( board, depth, alpha, beta, value, best_move_ptr, PV ) )
    {
        return value;
    }

    if ( board.get_player( ) == m_player )
    {
        return get_best_value( board, alpha, beta, depth, PV, best_move_ptr );
    }
    else if ( board.get_player( ) == m_teammate )
    {
        return get_average_value( board, alpha, beta, depth );
    }
    else
    {
        return get_worst_value( board, alpha, beta, depth, PV );
    }
}

Action
ExpectMinMaxStrategy::get_best_action( const Board& board )
{
    std::cerr << "Using ExpectMinMaxStrategy\n";

    double last_total_time = 0.0;
    double total_time = 0.0;
    const double max_turn_time = Timer::get_max_time( board );

    int32_t depth = START_DEPTH;
    for ( ; depth <= MAX_DEPTH; ++depth )
    {
        m_can_abort = depth > MIN_DEPTH;
        Move best_move = INVALID_MOVE;
        init_search( );
        Timer timer;
        double value = search( board, -OO, +OO, depth, &best_move );
        timer.stop( );
        double dt = timer.get_delta_time( );
        total_time += dt;
        std::cerr << "\td=" << depth << " v=" << value << " n=" << m_nodes << " c=" << m_cutoff
                  << " dt=" << dt << " tt=" << total_time << " ttt=" << timer.get_total_time( )
                  << "\n";

        if ( m_aborted )
        {
            std::cerr << "aborted!\n";
            break;
        }

        if ( depth >= MIN_DEPTH )
        {
            double branching_factor = DEFAULT_BRANCHING_FACTOR;
            if ( last_total_time > 0 )
            {
                branching_factor = total_time;
                branching_factor /= last_total_time;
            }

            if ( branching_factor * total_time >= max_turn_time )
            {
                break;
            }
        }

        last_total_time = total_time;
    }

    std::cerr << "Constructing best moves list\n";

    if ( m_aborted or depth > MAX_DEPTH )
    {
        --depth;
    }

    m_can_abort = false;
    Action best_action;
    for ( Board next_board = board; next_board.get_player( ) == m_player; )
    {
        Move best_move = INVALID_MOVE;
        init_search( );
        Timer timer;
        double value = search( next_board, -OO, +OO, depth, &best_move );
        timer.stop( );
        double dt = timer.get_delta_time( );
        total_time += dt;
        std::cerr << "\td=" << depth << " v=" << value << " n=" << m_nodes << " c=" << m_cutoff
                  << " dt=" << dt << " tt=" << total_time << " ttt=" << timer.get_total_time( )
                  << std::endl;

        if ( best_move != NIL_MOVE )
        {
            best_action.push_back( best_move );
            next_board.do_move( best_move );
        }
        else
        {
            break;
        }
    }

    std::cerr << Conversion::action_to_string( best_action ) << std::endl;

    return best_action;
}



RandomStrategy::RandomStrategy( )
{
}

RandomStrategy::~RandomStrategy( )
{
}

Action
RandomStrategy::get_best_action( const Board& board )
{
    Action best_action;
    const auto player = board.get_player( );
    Board next_board = board;

    while ( next_board.get_player( ) == player )
    {
        const auto random_move = next_board.get_random_move( );

        if ( random_move != NIL_MOVE )
        {
            best_action.push_back( random_move );
        }
        else
        {
            break;
        }

        next_board.do_move( random_move );
    }

    return best_action;
}


RunStrategy::RunStrategy( )
{
}

RunStrategy::~RunStrategy( )
{
}

Action
RunStrategy::get_best_action( const Board& board )
{
    Action best_action;

    Timer timer;
    board.display( );

    auto next_board = board;
    auto player = board.get_player( );
    auto cost = 0;
    while ( not next_board.is_done( player ) )
    {
        auto best_move = next_board.get_best_running_move( );
        best_action.push_back( best_move );
        cost += next_board.do_move( best_move );
    }

    timer.stop( );

    std::cerr << "Using RunStrategy\n\tcost=" << cost << " dt=" << timer.get_delta_time( )
              << " tt=" << Timer::get_total_time( ) << "\n";

    return best_action;
}


Move
Conversion::string_to_move( const std::string& s )
{
    if ( s == "Nil" )
    {
        return NIL_MOVE;
    }

    const int32_t from_line = s[ 0 ] - 'a';
    const int32_t from_column = s[ 1 ] - '1';
    const int32_t from = ( from_line << 3 ) + from_column;
    const int32_t to_line = s[ 2 ] - 'a';
    const int32_t to_column = s[ 3 ] - '1';
    const int32_t to = ( to_line << 3 ) + to_column;
    const Move move = from + ( to << 6 );

    return move;
}

std::string
Conversion::move_to_string( const Move move )
{
    if ( move == NIL_MOVE )
    {
        return std::string( "Nil" );
    }

    if ( move == INVALID_MOVE )
    {
        return std::string( "Invalid" );
    }

    const int32_t from = move & 0x3f;
    const int32_t to = ( move >> 6 ) & 0x3f;
    const char from_line = 'a' + ( from >> 3 );
    const char from_column = '1' + ( from & 7 );
    const char to_line = 'a' + ( to >> 3 );
    const char to_column = '1' + ( to & 7 );
    std::ostringstream stream;
    stream << from_line << from_column << to_line << to_column;

    return stream.str( );
}

Action
Conversion::string_to_action( const std::string& s )
{
    const char delimiter = ':';

    std::istringstream iss( s );
    std::string token;
    Action action;
    while ( std::getline( iss, token, delimiter ) )
    {
        action.push_back( string_to_move( token ) );
    }

    return action;
}

std::string
Conversion::action_to_string( const Action& action )
{
    if ( action.empty( ) )
    {
        return std::string( "Nil" );
    }

    std::ostringstream stream;
    bool first = true;
    for ( const Move move : action )
    {
        if ( first )
        {
            first = false;
        }
        else
        {
            stream << ":";
        }

        stream << move_to_string( move );
    }

    return stream.str( );
}


namespace
{
const double UCTK = 0.1;

constexpr int MAX_VISITS = 32000;

double SQRT_LOG[ MAX_VISITS ];
double SQRT[ MAX_VISITS ];

std::list< Node* > allocated_nodes;

void
update_stats( Node::Stats& stats, const double new_value )
{
    double& value = stats.value;
    int32_t& visits = stats.visits;
    value = ( value * visits + new_value ) / ( visits + 1 );
    visits += 1;
}

uint64_t
get_combined_hash( const uint64_t hash, const uint64_t lock )
{
    return hash | ( lock << 32 );
}

void
destroy_allocated_nodes( )
{
    for ( Node* node : allocated_nodes )
    {
        delete node;
    }

    allocated_nodes.clear( );
}

std::vector< Node::SharedStats > shared_stats{MAX_VISITS};
Node::SharedStats* next_shared_stats = &shared_stats.front( );
std::unordered_map< uint64_t, Node::SharedStats* > shared_stats_map{MAX_VISITS};

int32_t transpositions = 0;
Node::SharedStats*
get_or_create_shared_stats( const Board& board, bool& new_stats )
{
    const uint64_t hash = board.get_hash( );
    const uint64_t lock = board.get_lock( );
    const uint64_t combined_hash = get_combined_hash( hash, lock );
    const auto iterator = shared_stats_map.find( combined_hash );
    if ( iterator != shared_stats_map.cend( ) )
    {
        ++transpositions;
        new_stats = false;
        return iterator->second;
    }

    new_stats = true;
    Node::SharedStats* stats = next_shared_stats++;
    stats->mc = Node::Stats{};
    stats->rave.clear( );
    shared_stats_map.emplace( combined_hash, stats );

    return stats;
}
}

Node::Node( const Board& board,
            const Move move,
            Node* parent,
            const double weight,
            const double bias )
    : player( board.get_player( ) )
    , move( move )
    , parent( parent )
    , weight( weight )
    , bias( bias )
    , value( 0.0 )
    , visits( 0 )
    , is_leaf( true )
{
    if ( parent == nullptr )
    {
        level = 0;
    }
    else if ( parent->player == player )
    {
        level = parent->level;
    }
    else
    {
        level = parent->level + 1;
    }

    bool new_stats;
    stats = get_or_create_shared_stats( board, new_stats );

    if ( not board.end_game( ) )
    {
        if ( board.is_running( player ) )
        {
            const Move running_move = board.get_best_running_move( );
            untried_moves.push_back( running_move );
            is_leaf = false;
            stats->rave[ running_move ] = Stats( );
        }
        else
        {
            Move sorted_moves[ MAX_MOVES ];
            auto sorted_moves_end = board.get_sorted_moves( sorted_moves );
            untried_moves = std::list< Move >( sorted_moves, sorted_moves_end );
            is_leaf = untried_moves.empty( );
            if ( new_stats )
            {
                std::for_each( sorted_moves, sorted_moves_end,
                               [&]( Move m ) { stats->rave.emplace( m, Stats{} ); } );
            }
        }
    }

    allocated_nodes.push_back( this );
}

Node::~Node( )
{
}

Node*
Node::expand( Board& board )
{
    const auto move = untried_moves.front( );
    untried_moves.pop_front( );
    const double score = board.get_score( player );
    board.do_move( move );
    double delta_score = board.get_score( player ) - score;
    const double weight = std::pow( 16.0, delta_score );
    const double bias = 0.05 * delta_score;
    auto child = new Node( board, move, this, weight, bias );
    children.emplace_back( child );

    return child;
}

double
Node::get_exploration_bonus( const Node* child ) const
{
    return UCTK * SQRT_LOG[ visits ] / SQRT[ child->visits ] + child->bias / child->visits;
}

void
Node::update_value( )
{
    if ( parent == nullptr )
    {
        value = stats->mc.value;
        return;
    }

    const auto& node_rave_stats = parent->stats->rave[ move ];
    const auto& node_mc_stats = stats->mc;
    const int32_t m = node_rave_stats.visits;
    const int32_t n = node_mc_stats.visits;
    double b = node_mc_stats.visits >= 100 ? node_rave_stats.value - node_mc_stats.value : 0.0;
    const double beta = static_cast< double >( m ) / ( m + n + 4 * b * b * m * n );
    value = ( 1.0 - beta ) * node_mc_stats.value + beta * node_rave_stats.value;
}

Node*
Node::select_best( )
{
    Node* best_child = nullptr;
    double best_value = -OO;

    for ( Node* child : children )
    {
        const double child_value = child->value + get_exploration_bonus( child );
        if ( best_value < child_value )
        {
            best_value = child_value;
            best_child = child;
        }
    }

    return best_child;
}

Node*
Node::select_worst( )
{
    Node* worst_child = nullptr;
    double worst_value = OO;

    for ( Node* child : children )
    {
        double child_value = child->value - get_exploration_bonus( child );
        if ( worst_value > child_value )
        {
            worst_value = child_value;
            worst_child = child;
        }
    }

    return worst_child;
}

Node*
Node::select_randomly( )
{
    using WeightedChild = std::pair< double, Node* >;

    WeightedChild weighted_children[ children.size( ) ];
    WeightedChild* weighted_children_end = weighted_children;

    double sum_weights = 0.0;
    for ( Node* child : children )
    {
        sum_weights += child->weight;
        weighted_children_end->first = sum_weights;
        weighted_children_end->second = child;

        ++weighted_children_end;
    }

    const auto random_weight = RandomNumberGenerator::pick( sum_weights );
    const WeightedChild* iterator = std::upper_bound( weighted_children, weighted_children_end,
                                                      WeightedChild{random_weight, nullptr} );

    return iterator->second;
}

Node*
Node::select( const Board::Player reference )
{
    if ( player == reference )
    {
        return select_best( );
    }
    else if ( player == Board::get_teammate( reference ) )
    {
        return select_randomly( );
    }
    else
    {
        return select_worst( );
    }
}

Node*
Node::select_most_visited( )
{
    Node* most_visited = nullptr;
    int32_t max_visits = 0;

    for ( Node* child : children )
    {
        if ( max_visits < child->visits )
        {
            max_visits = child->visits;
            most_visited = child;
        }
    }

    return most_visited;
}

void
Node::update( )
{
    ++visits;

    update_value( );
}

void
Node::mc_update( const double new_value )
{
    update_stats( stats->mc, new_value );
}

void
Node::rave_update( const Board::Player p, const Move move, const double new_value )
{
    if ( player != p )
    {
        return;
    }

    if ( move == NIL_MOVE )
    {
        return;
    }

    const auto iterator = stats->rave.find( move );
    if ( iterator == stats->rave.cend( ) )
    {
        return;
    }

    update_stats( iterator->second, new_value );
}

bool
Node::is_fully_expanded( ) const
{
    return untried_moves.empty( );
}

void
Node::init_data( )
{
    for ( auto v = 0; v < MAX_VISITS; ++v )
    {
        SQRT_LOG[ v ] = std::sqrt( std::log( v ) );
        SQRT[ v ] = std::sqrt( v );
    }
}

void
Node::clear_all_nodes( )
{
    destroy_allocated_nodes( );
    next_shared_stats = &shared_stats.front( );
    std::cerr << "\ttc=" << transpositions << std::endl;
    transpositions = 0;
}


namespace
{
const int32_t MAX_LEVEL = 4;

Move
get_default_policy_move( const Board& board )
{
    // TODO: prevent next player jumps
    return board.get_random_move( );
}

bool
check_consistency( Node* root )
{
    const auto player = root->player;
    for ( auto node = root; not node->is_leaf and node->player == player; )
    {
        auto most_visited = node->select_most_visited( );
        auto best = node->select( player );

        if ( best != most_visited )
        {
            return false;
        }

        node = most_visited;
    }

    return true;
}

Action
extract_best_action( Node* root )
{
    const auto player = root->player;

    Action best_action;
    for ( auto node = root; not node->is_leaf and node->player == player; )
    {
        node = node->select_most_visited( );
        if ( node->move != NIL_MOVE )
        {
            best_action.push_back( node->move );
        }
        else
        {
            break;
        }
    }

    return best_action;
}

void
log_expected_variation( Node* root )
{
    std::cerr << "\tEV";

    int32_t depth = 0;
    for ( auto node = root; node->is_fully_expanded( ) and not node->is_leaf; ++depth )
    {
        node = node->select_most_visited( );
        std::cerr << "=>" << Conversion::move_to_string( node->move );
    }

    std::cerr << std::endl;
}

double
get_adapted_score( const double score )
{
    const double min_score = -10.0;
    const double max_score = 10.0;

    double adapted_score = score;

    if ( adapted_score < min_score )
    {
        adapted_score = min_score;
    }

    if ( adapted_score > max_score )
    {
        adapted_score = max_score;
    }

    adapted_score -= min_score;
    adapted_score /= max_score - min_score;

    return adapted_score;
}
}

MCTSStrategy::MCTSStrategy( const Board::Player player )
    : m_player( player )
{
}

MCTSStrategy::~MCTSStrategy( )
{
}

double
MCTSStrategy::run_simulation( Board& board,
                              const int32_t max_turns,
                              std::vector< PlayerMove >& moves )
{
    while ( board.get_turns( ) < max_turns and not board.is_running( ) )
    {
        const auto player = board.get_player( );
        const auto default_policy_move = get_default_policy_move( board );
        if ( default_policy_move == INVALID_MOVE )
        {
            break;
        }

        board.do_move( default_policy_move );
        moves.emplace_back( player, default_policy_move );
    }

    const double score = board.get_score( m_player );
    return get_adapted_score( score );
}

Action
MCTSStrategy::get_best_action( const Board& board )
{
    Timer timer;

    std::cerr << "Using MCTSStrategy\n";

    Node* root = new Node( board );

    const int32_t max_iterations = 30000;
    const int32_t max_check_iterations = 32000;
    const int32_t turns = board.get_turns( );
    const int32_t max_turns = std::min( 80, 4 * ( turns / 4 ) + 16 );
    int32_t iteration = 0;
    for ( ;; ++iteration )
    {
        auto node = root;
        Board tmp_board = board;
        std::vector< PlayerMove > moves;

        // selection
        while ( node->is_fully_expanded( ) and not node->is_leaf )
        {
            auto player = node->player;
            node = node->select( m_player );
            tmp_board.do_move( node->move );
            moves.emplace_back( player, node->move );
        }

        // expansion
        if ( node->level < MAX_LEVEL and not node->is_fully_expanded( ) )
        {
            auto previous_node = node;
            node = node->expand( tmp_board );
            moves.emplace_back( previous_node->player, node->move );
        }

        // simulation
        const double value = run_simulation( tmp_board, max_turns, moves );

        // back propagate
        while ( node != nullptr )
        {
            // Monte Carlo update
            node->mc_update( value );

            // RAVE update
            for ( size_t level = node->level; level < moves.size( ); ++level )
            {
                const PlayerMove& data = moves[ level ];
                node->rave_update( data.player, data.move, value );
            }

            node->update( );

            node = node->parent;
        }

        if ( iteration >= max_iterations and check_consistency( root ) )
        {
            break;
        }

        if ( iteration >= max_check_iterations )
        {
            break;
        }
    }

    const auto best_action = extract_best_action( root );
    log_expected_variation( root );

    auto most_visited = root->select_most_visited( );

    const auto& mc_stats = most_visited->stats->mc;
    const auto& rave_stats = root->stats->rave[ most_visited->move ];
    std::cerr << "\ti=" << iteration << " w=" << most_visited->weight << " mc=(" << mc_stats.visits
              << ", " << mc_stats.value << ") rave=(" << rave_stats.visits << ", "
              << rave_stats.value << ")\n";

    Node::clear_all_nodes( );

    timer.stop( );

    std::cerr << "\tdt=" << timer.get_delta_time( ) << " tt=" << timer.get_total_time( ) << "\n\t"
              << Conversion::action_to_string( best_action ) << std::endl;

    return best_action;
}


using AdoptedStrategy = MCTSStrategy;

namespace
{
Board::Player
get_player( const std::string& color )
{
    if ( color == "Yellow" )
    {
        return Board::YELLOW;
    }
    else if ( color == "Black" )
    {
        return Board::BLACK;
    }
    else if ( color == "White" )
    {
        return Board::WHITE;
    }
    else
    {
        return Board::RED;
    }
}

int
usage( )
{
    std::cout << "Player or\n";
    std::cout << "Player --test-run-strategy or\n";
    std::cout << "Player --test-random-move or\n";
    std::cout << "Player --analyze\n";

    return 1;
}

int
test_run_strategy( )
{
    std::string layout;
    std::cin >> layout;

    Board::init_data( layout );
    Board::pre_compute( );

    Board board;
    board.enable_solo_mode( Board::YELLOW );

    const auto best_action = RunStrategy( ).get_best_action( board );
    for ( const auto move : best_action )
    {
        board.do_move( move );
        std::cerr << Conversion::move_to_string( move ) << std::endl;
        board.display( );
    }

    return 0;
}

int
test_random_move( )
{
    std::string layout;
    std::cin >> layout;
    Board::init_data( layout );
    Board::pre_compute( );
    Board board;
    std::ostringstream stream;
    int32_t moves_count = 0;
    auto previous_player = Board::RED;
    std::cerr << "random game moves\n";
    while ( not board.end_game( ) )
    {
        const auto random_move = board.get_random_move( );
        const auto player = board.get_player( );
        if ( player == previous_player )
        {
            std::cerr << ":";
        }
        else
        {
            std::cerr << "\n";
            previous_player = player;
        }

        std::cerr << Conversion::move_to_string( random_move );

        board.do_move( random_move );
        ++moves_count;
    }

    std::cerr << "\nmoves-count=" << moves_count << std::endl;
    board.display( );

    return 0;
}

int
analyze( )
{
    std::string walls;
    std::cin >> walls;

    Board::init_data( walls );
    Board::pre_compute( );
    Node::init_data( );

    Board board;

    std::string s;
    for ( std::cin >> s; s != "End"; std::cin >> s )
    {
        const auto action = Conversion::string_to_action( s );
        board.do_action( action );
    }

    board.display( );
    const auto best_action = AdoptedStrategy( board.get_player( ) ).get_best_action( board );
    std::cerr << Conversion::action_to_string( best_action ) << std::endl;

    return 0;
}

int
compare_strategies( )
{
    std::string walls;
    std::cin >> walls;

    Board::init_data( walls );
    Board::pre_compute( );
    Node::init_data( );

    std::vector< double > scores;
    const Board::Player players[] = {Board::YELLOW, Board::BLACK, Board::WHITE, Board::RED};
    for ( int i = 0; i < 100; ++i )
    {
        Board::Player player = players[ i % 4 ];
        Board board;
        while ( not board.end_game( ) )
        {
            Action action;
            const auto p = board.get_player( );
            if ( p == player )
            {
                action = MCTSStrategy( p ).get_best_action( board );
            }
            else
            {
                action = ExpectMinMaxStrategy( p ).get_best_action( board );
            }

            board.do_action( action );
        }
        double score = board.get_score( player );
        if ( score < -10.0 )
        {
            score = -10.0;
        }
        if ( score > +10.0 )
        {
            score = +10.0;
        }
        std::cout << "score=" << score << std::endl;
        scores.push_back( score );
    }

    std::cout << "average-score="
              << 0.01 * std::accumulate( scores.cbegin( ), scores.cend( ), 0.0,
                                         std::plus< double >( ) )
              << std::endl;

    return 0;
}

int
main_loop( )
{
    std::string walls;
    std::cin >> walls;
    std::cerr << "walls=" << walls << std::endl;

    Board::init_data( walls );
    Board::pre_compute( );
    Node::init_data( );

    Board board;

    // read color
    std::string color;
    std::cin >> color;
    std::cerr << "color=" << color << std::endl;

    const auto me = get_player( color );

    while ( true )
    {
        // read previous moves if any
        while ( board.get_player( ) != me )
        {
            std::string s;
            std::cin >> s;
            if ( s == "Quit" )
            {
                return 0;
            }
            else if ( s == "Move" )
            {
                board.enable_solo_mode( me );
                const auto action = RunStrategy( ).get_best_action( board );
                for ( const auto move : action )
                {
                    std::cout << Conversion::move_to_string( move ) << std::endl;
                }

                return 0;
            }
            else
            {
                std::cerr << "\t" << s << std::endl;
                const auto action = Conversion::string_to_action( s );
                board.do_action( action );
            }
        }

        const auto best_action = AdoptedStrategy{me}.get_best_action( board );
        board.do_action( best_action );
        std::cout << Conversion::action_to_string( best_action ) << std::endl;
    }
}
}

int
main( int argc, char** argv )
{
    RandomNumberGenerator::randomize( );

    if ( argc > 1 )
    {
        if ( std::string( "--test-run-strategy" ) == argv[ 1 ] )
        {
            return test_run_strategy( );
        }
        else if ( std::string( "--analyze" ) == argv[ 1 ] )
        {
            return analyze( );
        }
        else if ( std::string( "--test-random-move" ) == argv[ 1 ] )
        {
            return test_random_move( );
        }
        else if ( std::string( "--compare-strategies" ) == argv[ 1 ] )
        {
            return compare_strategies( );
        }
        else
        {
            return usage( );
        }
    }
    else
    {
        return main_loop( );
    }
}
