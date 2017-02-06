#include "Common.h"

#include "Board.h"
#include "Conversion.h"
#include "MCTSStrategy.h"
#include "Node.h"
#include "Timer.h"

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
