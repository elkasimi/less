#pragma once

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
