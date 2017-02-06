#include "Common.h"

#include "Board.h"
#include "Node.h"
#include "RandomNumberGenerator.h"

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
