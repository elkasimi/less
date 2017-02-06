#include "Common.h"

#include "ExpectMinMaxStrategy.h"

#include "Board.h"
#include "Conversion.h"
#include "Timer.h"

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
