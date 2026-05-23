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

#include "../include/RBD_DB.hpp"
#include <gtest/gtest.h>

TEST(REBL, read_rbd_db)
{
    REBL::DB::Connection rbd_db("../tests/rbd.db");
    auto rbd = rbd_db.get_rbd();
    EXPECT_EQ(rbd.at(0), "MGCC");
    EXPECT_EQ(rbd.at(1), "Actor_(1,1)->MGCC");
    EXPECT_EQ(rbd.at(2), "Actor_(m_1,1)->MGCC");
    EXPECT_EQ(rbd.at(3).at(0).at(0).at(0).at(0), "Sensor_1");
    EXPECT_EQ(rbd.at(3).at(0).at(0).at(0).at(1), "Sensor_1->Actor_(1,1)");
    EXPECT_EQ(rbd.at(3).at(0).at(0).at(1).at(0), "Sensor_n");
    EXPECT_EQ(rbd.at(3).at(0).at(0).at(1).at(1), "Sensor_n->Actor_(1,1)");
    EXPECT_TRUE(rbd.at(3).at(0).at(1).is_null());
    EXPECT_EQ(rbd.at(3).at(0).at(2).at(0).at(0), "Sensor_1");
    EXPECT_EQ(rbd.at(3).at(0).at(2).at(0).at(1), "Sensor_1->Actor_(m_1,1)");
    EXPECT_EQ(rbd.at(3).at(0).at(2).at(1).at(0), "Sensor_n");
    EXPECT_EQ(rbd.at(3).at(0).at(2).at(1).at(1), "Sensor_n->Actor_(m_1,1)");
    EXPECT_EQ(rbd.at(3).at(0).at(3), "Actor_(1,1)");
    EXPECT_EQ(rbd.at(3).at(1).at(0).at(0), "Sensor_1");
    EXPECT_EQ(rbd.at(3).at(1).at(0).at(1), "Sensor_1->SCC_1");
    EXPECT_EQ(rbd.at(3).at(1).at(0).at(2), "SCC_1");
    EXPECT_EQ(rbd.at(3).at(1).at(0).at(3), "SCC_1->MGCC");
    EXPECT_EQ(rbd.at(3).at(1).at(1).at(0), "Sensor_n");
    EXPECT_EQ(rbd.at(3).at(1).at(1).at(1), "Sensor_n->SCC_n");
    EXPECT_EQ(rbd.at(3).at(1).at(1).at(2), "SCC_n");
    EXPECT_EQ(rbd.at(3).at(1).at(1).at(3), "SCC_n->MGCC");
    EXPECT_EQ(rbd.at(3).at(1).at(2), "Actor_(1,2)->MGCC");
    EXPECT_EQ(rbd.at(3).at(1).at(3), "Actor_(m_1,2)->MGCC");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(0), "ACC_(1,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(1), "MGCC->ACC_(1,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(2), "ACC_(1,2)->Actor_(1,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(3), "Actor_(1,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(4), "ACC_(m_2,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(5), "MGCC->ACC_(m_2,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(6), "ACC_(m_2,2)->Actor_(m_2,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(0).at(7), "Actor_(m_2,2)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(0), "Actor_(1,p)->MGCC");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(1), "Actor_(m_p,p)->MGCC");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(2), "ACC_(1,p)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(3), "MGCC->ACC_(1,p)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(4), "ACC_(1,p)->Actor_(1,p)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(5), "Actor_(1,p)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(6), "ACC_(m_p,p)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(7), "MGCC->ACC_(m_p,p)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(8), "ACC_(m_p,p)->Actor_(m_p,p)");
    EXPECT_EQ(rbd.at(3).at(1).at(4).at(1).at(9), "Actor_(m_p,p)");
    // auto components = rbd_db.get_components();
}

#endif // _REBL_TEST_HPP