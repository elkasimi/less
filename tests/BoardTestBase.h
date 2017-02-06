#pragma once

#include <gtest/gtest.h>

#include "../player/Common.h"

#include "../player/Board.h"

class BoardTestBase : public Board, public ::testing::Test
{
public:
    explicit BoardTestBase(const std::string& layout);
    virtual ~BoardTestBase();

    void SetUp() override;
    void TearDown() override;

    std::string m_layout;
};
