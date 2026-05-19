/**
 * Tests for REBL - Library for handling and analysis of REliability BLock
 * diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_TEST_HPP
#define _REBL_TEST_HPP

#include "../include/REBL.hpp"
#include <graaflib/graph.h>
#include <gtest/gtest.h>

TEST(REBL, graaf_dependency)
{
    graaf::directed_graph<const char, int> g;
    const auto a = g.add_vertex('a');
    const auto b = g.add_vertex('b');
    const auto c = g.add_vertex('c');

    g.add_edge(a, b, 1);
    g.add_edge(a, c, 1);
}

#endif // _REBL_TEST_HPP