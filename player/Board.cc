#include "Common.h"

#include "Board.h"
#include "RandomNumberGenerator.h"
#include "Timer.h"

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
