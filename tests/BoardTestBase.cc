#include "BoardTestBase.h"

BoardTestBase::BoardTestBase( const std::string& layout )
    : m_layout( layout )
{
}

BoardTestBase::~BoardTestBase( )
{
}

void
BoardTestBase::SetUp( )
{
    ::testing::Test::SetUp( );
    Board::init_data( m_layout );
}

void
BoardTestBase::TearDown( )
{
    ::testing::Test::TearDown( );
}
