#pragma once

#include "Board.h"
#include "Common.h"

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
