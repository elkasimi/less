#include "Common.h"

#include "Conversion.h"

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
