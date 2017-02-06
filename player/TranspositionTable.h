#pragma once

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
