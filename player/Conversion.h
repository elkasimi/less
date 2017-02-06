#pragma once

class Conversion
{
public:
    static Move string_to_move( const std::string& s );
    static std::string move_to_string( const Move move );
    static Action string_to_action( const std::string& s );
    static std::string action_to_string( const Action& action );
};
