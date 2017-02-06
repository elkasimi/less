#include "Common.h"

#include "Board.h"
#include "Conversion.h"
#include "ExpectMinMaxStrategy.h"
#include "MCTSStrategy.h"
#include "Node.h"
#include "RandomNumberGenerator.h"
#include "RunStrategy.h"

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
