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
#include "../include/SCV.hpp"
#include "../include_private/FC.hpp"
#include "../include_private/RBD_DB.hpp"
#include <filesystem>
#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void expect_rel_near(double actual, double expected, double rel_tol)
{
    double diff  = std::abs(actual - expected);
    double scale = std::max(std::abs(actual), std::abs(expected));

    EXPECT_LE(diff, rel_tol * scale)
        << "actual=" << actual << " expected=" << expected;
}

TEST(REBL, read_rbd_db)
{
    REBL::DB::Connection rbd_db(std::string(TEST_DATA_DIR) + "/rbd.db");
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
    auto components = rbd_db.get_rbd_components();
    EXPECT_FLOAT_EQ(components.at("Actor_(m_p,p)").H(), 0.021);
    EXPECT_FLOAT_EQ(components.at("Actor_(m_p,p)").T(), 0.0001632345);
    EXPECT_FLOAT_EQ(components.at("Sensor_1").H(), 0.1);
    EXPECT_FLOAT_EQ(components.at("Sensor_1").T(), 0.00057074999999999997);
    EXPECT_FLOAT_EQ(components.at("MGCC").H(), 0.0);
    EXPECT_FLOAT_EQ(components.at("MGCC").T(), 0.0);
    EXPECT_FLOAT_EQ(components.at("Sensor_1->Actor_(1,1)").H(),
                    4.8799999999999999);
    EXPECT_FLOAT_EQ(components.at("Sensor_1->Actor_(1,1)").T(),
                    0.0079904999999999993);
    EXPECT_FLOAT_EQ(components.at("SCC_n->MGCC").H(), 0.022000000000000002);
    EXPECT_FLOAT_EQ(components.at("SCC_n->MGCC").T(), 0.013698);
}

std::string sort_lines_helper(std::string input)
{
    // This helper function is AI generated. It is not part of the actual
    // codebase.
    std::istringstream iss(input);
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(iss, line))
    {
        lines.push_back(line);
    }

    std::sort(lines.begin(), lines.end());

    std::ostringstream oss;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (i != 0)
            oss << '\n';
        oss << lines[i];
    }

    return oss.str();
}

TEST(REBL, parse_rbd_db)
{
    // This parses in the constructor.
    REBL::RBD rebl(std::string(TEST_DATA_DIR) + "/rbd.db", REBL::MCSSettings());
    auto graph                  = rebl.get_pure_graph();
    auto graph_adjacency_string = rebl.get_graph_adjacency_string(true, true);
    std::string expected_graph_adjacency_string =
        R"(61 (Actor_(m_p,p)) -> [ 43 (§virtual§)] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
60 (ACC_(m_p,p)->Actor_(m_p,p)) -> [ 61 (Actor_(m_p,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
28 (§virtual§) -> [ 30 (Sensor_1), 34 (Sensor_n)] (H: 0; T: 0; P: 0)
29 (§virtual§) -> [ 38 (Actor_(1,2)->MGCC)] (H: 0; T: 0; P: 0)
27 (Actor_(1,1)) -> [ 9 (§virtual§)] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
26 (Sensor_n->Actor_(m_1,1)) -> [ 22 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
25 (Sensor_n) -> [ 26 (Sensor_n->Actor_(m_1,1))] (H: 0.001; T: 0.0004566; P: 4.566e-07)
24 (Sensor_1->Actor_(m_1,1)) -> [ 22 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
23 (Sensor_1) -> [ 24 (Sensor_1->Actor_(m_1,1))] (H: 0.1; T: 0.00057075; P: 5.7075e-05)
21 (§virtual§) -> [ 23 (Sensor_1), 25 (Sensor_n)] (H: 0; T: 0; P: 0)
22 (§virtual§) -> [ 20 (§virtual§)] (H: 0; T: 0; P: 0)
19 (§virtual§) -> [ 21 (§virtual§)] (H: 0; T: 0; P: 0)
20 (§virtual§) -> [ 27 (Actor_(1,1))] (H: 0; T: 0; P: 0)
18 (§virtual§) -> [ 19 (§virtual§)] (H: 0; T: 0; P: 0)
17 (Sensor_n->Actor_(1,1)) -> [ 13 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
16 (Sensor_n) -> [ 17 (Sensor_n->Actor_(1,1))] (H: 0.001; T: 0.0004566; P: 4.566e-07)
15 (Sensor_1->Actor_(1,1)) -> [ 13 (§virtual§)] (H: 4.88; T: 0.0079905; P: 0.0389936)
14 (Sensor_1) -> [ 15 (Sensor_1->Actor_(1,1))] (H: 0.1; T: 0.00057075; P: 5.7075e-05)
1 (§virtual§) -> [ 3 (MGCC)] (H: 0; T: 0; P: 0)
2 (§virtual§) -> [ ] (H: 0; T: 0; P: 0)
3 (MGCC) -> [ 4 (Actor_(1,1)->MGCC)] (H: 0; T: 0; P: 0)
4 (Actor_(1,1)->MGCC) -> [ 5 (Actor_(m_1,1)->MGCC)] (H: 0.022; T: 0.013698; P: 0.000301356)
5 (Actor_(m_1,1)->MGCC) -> [ 6 (§virtual§)] (H: 0; T: 0; P: 0)
7 (§virtual§) -> [ 2 (§virtual§)] (H: 0; T: 0; P: 0)
6 (§virtual§) -> [ 8 (§virtual§)] (H: 0; T: 0; P: 0)
9 (§virtual§) -> [ 7 (§virtual§)] (H: 0; T: 0; P: 0)
8 (§virtual§) -> [ 10 (§virtual§), 28 (§virtual§)] (H: 0; T: 0; P: 0)
11 (§virtual§) -> [ 18 (§virtual§)] (H: 0; T: 0; P: 0)
10 (§virtual§) -> [ 12 (§virtual§)] (H: 0; T: 0; P: 0)
13 (§virtual§) -> [ 11 (§virtual§)] (H: 0; T: 0; P: 0)
12 (§virtual§) -> [ 14 (Sensor_1), 16 (Sensor_n)] (H: 0; T: 0; P: 0)
30 (Sensor_1) -> [ 31 (Sensor_1->SCC_1)] (H: 0.1; T: 0.00057075; P: 5.7075e-05)
31 (Sensor_1->SCC_1) -> [ 32 (SCC_1)] (H: 0.022; T: 0.013698; P: 0.000301356)
32 (SCC_1) -> [ 33 (SCC_1->MGCC)] (H: 0; T: 0; P: 0)
33 (SCC_1->MGCC) -> [ 29 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
34 (Sensor_n) -> [ 35 (Sensor_n->SCC_n)] (H: 0.001; T: 0.0004566; P: 4.566e-07)
35 (Sensor_n->SCC_n) -> [ 36 (SCC_n)] (H: 0.022; T: 0.013698; P: 0.000301356)
36 (SCC_n) -> [ 37 (SCC_n->MGCC)] (H: 0; T: 0; P: 0)
37 (SCC_n->MGCC) -> [ 29 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
38 (Actor_(1,2)->MGCC) -> [ 39 (Actor_(m_1,2)->MGCC)] (H: 0.022; T: 0.013698; P: 0.000301356)
39 (Actor_(m_1,2)->MGCC) -> [ 40 (§virtual§)] (H: 0.022; T: 0.013698; P: 0.000301356)
41 (§virtual§) -> [ 9 (§virtual§)] (H: 0; T: 0; P: 0)
40 (§virtual§) -> [ 42 (§virtual§)] (H: 0; T: 0; P: 0)
43 (§virtual§) -> [ 41 (§virtual§)] (H: 0; T: 0; P: 0)
42 (§virtual§) -> [ 44 (ACC_(1,2)), 52 (Actor_(1,p)->MGCC)] (H: 0; T: 0; P: 0)
44 (ACC_(1,2)) -> [ 45 (MGCC->ACC_(1,2))] (H: 0; T: 0; P: 0)
45 (MGCC->ACC_(1,2)) -> [ 46 (ACC_(1,2)->Actor_(1,2))] (H: 0.022; T: 0.013698; P: 0.000301356)
46 (ACC_(1,2)->Actor_(1,2)) -> [ 47 (Actor_(1,2))] (H: 0.022; T: 0.013698; P: 0.000301356)
47 (Actor_(1,2)) -> [ 48 (ACC_(m_2,2))] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
48 (ACC_(m_2,2)) -> [ 49 (MGCC->ACC_(m_2,2))] (H: 0; T: 0; P: 0)
49 (MGCC->ACC_(m_2,2)) -> [ 50 (ACC_(m_2,2)->Actor_(m_2,2))] (H: 0; T: 0; P: 0)
50 (ACC_(m_2,2)->Actor_(m_2,2)) -> [ 51 (Actor_(m_2,2))] (H: 0.022; T: 0.013698; P: 0.000301356)
51 (Actor_(m_2,2)) -> [ 43 (§virtual§)] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
52 (Actor_(1,p)->MGCC) -> [ 53 (Actor_(m_p,p)->MGCC)] (H: 0.022; T: 0.013698; P: 0.000301356)
53 (Actor_(m_p,p)->MGCC) -> [ 54 (ACC_(1,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
54 (ACC_(1,p)) -> [ 55 (MGCC->ACC_(1,p))] (H: 0; T: 0; P: 0)
55 (MGCC->ACC_(1,p)) -> [ 56 (ACC_(1,p)->Actor_(1,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
56 (ACC_(1,p)->Actor_(1,p)) -> [ 57 (Actor_(1,p))] (H: 0.022; T: 0.013698; P: 0.000301356)
57 (Actor_(1,p)) -> [ 58 (ACC_(m_p,p))] (H: 0.021; T: 0.000163234; P: 3.42792e-06)
58 (ACC_(m_p,p)) -> [ 59 (MGCC->ACC_(m_p,p))] (H: 0; T: 0; P: 0)
59 (MGCC->ACC_(m_p,p)) -> [ 60 (ACC_(m_p,p)->Actor_(m_p,p))] (H: 0.022; T: 0.013698; P: 0.000301356))";
    EXPECT_EQ(sort_lines_helper(graph_adjacency_string),
              sort_lines_helper(expected_graph_adjacency_string));
}

void print_vector_of_strings(std::vector<std::string> vec)
{
    for (auto &str : vec)
    {
        std::cout << str << "\n";
    }
}

TEST(REBL_FC_Machine, combination_size_min1_max3)
{
    std::map<std::string, std::vector<int>> component_instance_map;
    REBL::MCSSettings mcs_settings = {false, 1, 3, 0.0, 0.0};
    KnoKan::DirectedGraph<int, REBL::ComponentData, EmptyP> rbd;

    rbd.add_node(1, REBL::ComponentData(0.2, 2));
    component_instance_map["1"] = {1};
    rbd.add_node(2, REBL::ComponentData(0.2, 2));
    rbd.add_node(3, REBL::ComponentData(0.2, 2));
    component_instance_map["2"] = {2, 3};
    rbd.add_node(4, REBL::ComponentData(0.2, 2));
    component_instance_map["3"] = {4};
    rbd.add_node(5, REBL::ComponentData(0.2, 2));
    component_instance_map["4"] = {5};
    rbd.add_node(6);
    rbd.add_node(7, REBL::ComponentData(0.2, 2));
    component_instance_map["5"] = {7};
    rbd.add_node(8);
    component_instance_map["n6_ideal"] = {8};

    // BEGIN #1: this is technically not necessary, but for regression's sake
    // I'm leaving it in.
    rbd.add_edge(1, 2);
    rbd.add_edge(2, 3);
    rbd.add_edge(3, 4);
    rbd.add_edge(4, 5);
    rbd.add_edge(5, 6);
    rbd.add_edge(6, 7);
    rbd.add_edge(7, 8);
    // END #1

    REBL::FailureCombination::Machine fc_machine(component_instance_map,
                                                 mcs_settings,
                                                 rbd);

    std::vector<std::vector<std::string>> expected_sequence = {{"1"},
                                                               {"1", "2"},
                                                               {"1", "2", "3"},
                                                               {"1", "2", "4"},
                                                               {"1", "2", "5"},
                                                               {"1", "3"},
                                                               {"1", "3", "4"},
                                                               {"1", "3", "5"},
                                                               {"1", "4"},
                                                               {"1", "4", "5"},
                                                               {"1", "5"},
                                                               {"2"},
                                                               {"2", "3"},
                                                               {"2", "3", "4"},
                                                               {"2", "3", "5"},
                                                               {"2", "4"},
                                                               {"2", "4", "5"},
                                                               {"2", "5"},
                                                               {"3"},
                                                               {"3", "4"},
                                                               {"3", "4", "5"},
                                                               {"3", "5"},
                                                               {"4"},
                                                               {"4", "5"},
                                                               {"5"}};

    unsigned int ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence[ctr]);
    }
    EXPECT_EQ(ctr, 25);
}

TEST(REBL_FC_Machine, combination_size_min1_max3_v2)
{
    std::map<std::string, std::vector<int>> component_instance_map;
    REBL::MCSSettings mcs_settings = {false, 1, 3, 0.0, 0.0};
    KnoKan::DirectedGraph<int, REBL::ComponentData, EmptyP> rbd;

    rbd.add_node(8);
    component_instance_map["06_ideal"] = {8};
    rbd.add_node(1, REBL::ComponentData(0.2, 2));
    component_instance_map["1"] = {1};
    rbd.add_node(2, REBL::ComponentData(0.2, 2));
    rbd.add_node(3, REBL::ComponentData(0.2, 2));
    component_instance_map["2"] = {2, 3};
    rbd.add_node(4, REBL::ComponentData(0.2, 2));
    component_instance_map["3"] = {4};
    rbd.add_node(5, REBL::ComponentData(0.2, 2));
    component_instance_map["4"] = {5};
    rbd.add_node(6);
    rbd.add_node(7, REBL::ComponentData(0.2, 2));
    component_instance_map["5"] = {7};

    // BEGIN #1: this is technically not necessary, but for regression's sake
    // I'm leaving it in.
    rbd.add_edge(1, 2);
    rbd.add_edge(2, 3);
    rbd.add_edge(3, 4);
    rbd.add_edge(4, 5);
    rbd.add_edge(5, 6);
    rbd.add_edge(6, 7);
    rbd.add_edge(3, 8);
    // END #1

    REBL::FailureCombination::Machine fc_machine(component_instance_map,
                                                 mcs_settings,
                                                 rbd);

    std::vector<std::vector<std::string>> expected_sequence = {{"1"},
                                                               {"1", "2"},
                                                               {"1", "2", "3"},
                                                               {"1", "2", "4"},
                                                               {"1", "2", "5"},
                                                               {"1", "3"},
                                                               {"1", "3", "4"},
                                                               {"1", "3", "5"},
                                                               {"1", "4"},
                                                               {"1", "4", "5"},
                                                               {"1", "5"},
                                                               {"2"},
                                                               {"2", "3"},
                                                               {"2", "3", "4"},
                                                               {"2", "3", "5"},
                                                               {"2", "4"},
                                                               {"2", "4", "5"},
                                                               {"2", "5"},
                                                               {"3"},
                                                               {"3", "4"},
                                                               {"3", "4", "5"},
                                                               {"3", "5"},
                                                               {"4"},
                                                               {"4", "5"},
                                                               {"5"}};

    unsigned int ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence[ctr]);
    }
    EXPECT_EQ(ctr, 25);
}
TEST(REBL_FC_Machine, combination_size_min1_max2_small)
{
    std::map<std::string, std::vector<int>> component_instance_map;
    REBL::MCSSettings mcs_settings = {false, 1, 2, 0.0, 0.0};
    KnoKan::DirectedGraph<int, REBL::ComponentData, EmptyP> rbd;

    rbd.add_node(1, REBL::ComponentData(0.2, 2));
    component_instance_map["1"] = {1};
    rbd.add_node(2, REBL::ComponentData(0.2, 2));
    rbd.add_node(3, REBL::ComponentData(0.2, 2));
    component_instance_map["2"] = {2, 3};

    // BEGIN #1: this is technically not necessary, but for regression's sake
    // I'm leaving it in.
    rbd.add_edge(1, 2);
    rbd.add_edge(2, 3);
    // END #1

    REBL::FailureCombination::Machine fc_machine(component_instance_map,
                                                 mcs_settings,
                                                 rbd);

    std::vector<std::vector<std::string>> expected_sequence = {{"1"},
                                                               {"1", "2"},
                                                               {"2"}};

    unsigned int ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence[ctr]);
    }
    EXPECT_EQ(ctr, 3);
}

TEST(REBL_FC_Machine, combination_size_min2_max3)
{
    std::map<std::string, std::vector<int>> component_instance_map;
    REBL::MCSSettings mcs_settings = {false, 2, 3, 0.0, 0.0};
    KnoKan::DirectedGraph<int, REBL::ComponentData, EmptyP> rbd;

    rbd.add_node(1, REBL::ComponentData(0.2, 2));
    component_instance_map["1"] = {1};
    rbd.add_node(2, REBL::ComponentData(0.2, 2));
    rbd.add_node(3, REBL::ComponentData(0.2, 2));
    component_instance_map["2"] = {2, 3};
    rbd.add_node(4, REBL::ComponentData(0.2, 2));
    component_instance_map["3"] = {4};
    rbd.add_node(5, REBL::ComponentData(0.2, 2));
    component_instance_map["4"] = {5};
    rbd.add_node(6);
    rbd.add_node(7, REBL::ComponentData(0.2, 2));
    component_instance_map["5"] = {7};
    rbd.add_node(8);
    component_instance_map["n6_ideal"] = {8};

    // BEGIN #1: this is technically not necessary, but for regression's sake
    // I'm leaving it in.
    rbd.add_edge(1, 2);
    rbd.add_edge(2, 3);
    rbd.add_edge(3, 4);
    rbd.add_edge(4, 5);
    rbd.add_edge(5, 6);
    rbd.add_edge(6, 7);
    rbd.add_edge(7, 8);
    // END #1

    REBL::FailureCombination::Machine fc_machine(component_instance_map,
                                                 mcs_settings,
                                                 rbd);

    std::vector<std::vector<std::string>> expected_sequence = {

        {"1", "2"},      {"1", "2", "3"}, {"1", "2", "4"}, {"1", "2", "5"},
        {"1", "3"},      {"1", "3", "4"}, {"1", "3", "5"}, {"1", "4"},
        {"1", "4", "5"}, {"1", "5"},      {"2", "3"},      {"2", "3", "4"},
        {"2", "3", "5"}, {"2", "4"},      {"2", "4", "5"}, {"2", "5"},
        {"3", "4"},      {"3", "4", "5"}, {"3", "5"},      {"4", "5"}};

    unsigned int ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence[ctr]);
    }
    EXPECT_EQ(ctr, 20);
}

TEST(REBL_FC_Machine, probability_min0dec1_max1)
{
    std::map<std::string, std::vector<int>> component_instance_map;
    REBL::MCSSettings mcs_settings = {true, 0, 0, 0.1, 1.0};
    KnoKan::DirectedGraph<int, REBL::ComponentData, EmptyP> rbd;

    rbd.add_node(1, REBL::ComponentData(0.2, 2));
    component_instance_map["1"] = {1};
    rbd.add_node(2, REBL::ComponentData(0.2, 2));
    rbd.add_node(3, REBL::ComponentData(0.2, 2));
    component_instance_map["2"] = {2, 3};
    rbd.add_node(4, REBL::ComponentData(0.2, 2));
    component_instance_map["3"] = {4};
    rbd.add_node(5, REBL::ComponentData(0.2, 2));
    component_instance_map["4"] = {5};
    rbd.add_node(6);
    rbd.add_node(7, REBL::ComponentData(0.2, 2));
    component_instance_map["5"] = {7};
    rbd.add_node(8);
    component_instance_map["n6_ideal"] = {8};

    // BEGIN #1: this is technically not necessary, but for regression's sake
    // I'm leaving it in.
    rbd.add_edge(1, 2);
    rbd.add_edge(2, 3);
    rbd.add_edge(3, 4);
    rbd.add_edge(4, 5);
    rbd.add_edge(5, 6);
    rbd.add_edge(6, 7);
    rbd.add_edge(7, 8);
    // END #1

    REBL::FailureCombination::Machine fc_machine(component_instance_map,
                                                 mcs_settings,
                                                 rbd);

    std::vector<std::vector<std::string>> expected_sequence = {

        {"1"},
        {"1", "2"},
        {"1", "3"},
        {"1", "4"},
        {"1", "5"},
        {"2"},
        {"2", "3"},
        {"2", "4"},
        {"2", "5"},
        {"3"},
        {"3", "4"},
        {"3", "5"},
        {"4"},
        {"4", "5"},
        {"5"}};

    unsigned int ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence[ctr]);
    }
    EXPECT_EQ(ctr, 15);
}

TEST(REBL_FC_Machine, probability_min0dec1_max0dec3)
{
    std::map<std::string, std::vector<int>> component_instance_map;
    REBL::MCSSettings mcs_settings = {true, 0, 0, 0.1, 0.3};
    KnoKan::DirectedGraph<int, REBL::ComponentData, EmptyP> rbd;

    rbd.add_node(1, REBL::ComponentData(0.2, 2));
    component_instance_map["1"] = {1};
    rbd.add_node(2, REBL::ComponentData(0.2, 2));
    rbd.add_node(3, REBL::ComponentData(0.2, 2));
    component_instance_map["2"] = {2, 3};
    rbd.add_node(4, REBL::ComponentData(0.2, 2));
    component_instance_map["3"] = {4};
    rbd.add_node(5, REBL::ComponentData(0.2, 2));
    component_instance_map["4"] = {5};
    rbd.add_node(6);
    rbd.add_node(7, REBL::ComponentData(0.2, 2));
    component_instance_map["5"] = {7};
    rbd.add_node(8);
    component_instance_map["n6_ideal"] = {8};

    // BEGIN #1: this is technically not necessary, but for regression's sake
    // I'm leaving it in.
    rbd.add_edge(1, 2);
    rbd.add_edge(2, 3);
    rbd.add_edge(3, 4);
    rbd.add_edge(4, 5);
    rbd.add_edge(5, 6);
    rbd.add_edge(6, 7);
    rbd.add_edge(7, 8);
    // END #1

    REBL::FailureCombination::Machine fc_machine(component_instance_map,
                                                 mcs_settings,
                                                 rbd);

    std::vector<std::vector<std::string>> expected_sequence = {

        {"1", "2"},
        {"1", "3"},
        {"1", "4"},
        {"1", "5"},
        {"2", "3"},
        {"2", "4"},
        {"2", "5"},
        {"3", "4"},
        {"3", "5"},
        {"4", "5"}};

    unsigned int ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence[ctr]);
    }
    EXPECT_EQ(ctr, 10);
}

TEST(REBL_FC_Machine, probability_reset_combination_size)
{
    std::map<std::string, std::vector<int>> component_instance_map;
    REBL::MCSSettings mcs_settings = {true, 0, 0, 0.1, 0.3};
    KnoKan::DirectedGraph<int, REBL::ComponentData, EmptyP> rbd;

    rbd.add_node(1, REBL::ComponentData(0.2, 2));
    component_instance_map["1"] = {1};
    rbd.add_node(2, REBL::ComponentData(0.2, 2));
    rbd.add_node(3, REBL::ComponentData(0.2, 2));
    component_instance_map["2"] = {2, 3};
    rbd.add_node(4, REBL::ComponentData(0.2, 2));
    component_instance_map["3"] = {4};
    rbd.add_node(5, REBL::ComponentData(0.2, 2));
    component_instance_map["4"] = {5};
    rbd.add_node(6);
    rbd.add_node(7, REBL::ComponentData(0.2, 2));
    component_instance_map["5"] = {7};
    rbd.add_node(8);
    component_instance_map["n6_ideal"] = {8};

    // BEGIN #1: this is technically not necessary, but for regression's sake
    // I'm leaving it in.
    rbd.add_edge(1, 2);
    rbd.add_edge(2, 3);
    rbd.add_edge(3, 4);
    rbd.add_edge(4, 5);
    rbd.add_edge(5, 6);
    rbd.add_edge(6, 7);
    rbd.add_edge(7, 8);
    // END #1

    REBL::FailureCombination::Machine fc_machine(component_instance_map,
                                                 mcs_settings,
                                                 rbd);

    std::vector<std::vector<std::string>> expected_sequence = {

        {"1", "2"},
        {"1", "3"},
        {"1", "4"},
        {"1", "5"},
        {"2", "3"},
        {"2", "4"},
        {"2", "5"},
        {"3", "4"},
        {"3", "5"},
        {"4", "5"},
    };

    unsigned int ctr = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence[ctr]);
    }
    EXPECT_NE(ctr, 0);

    auto fc = fc_machine.next();
    EXPECT_EQ(fc, expected_sequence[0]);
    fc = fc_machine.next();
    EXPECT_EQ(fc, expected_sequence[1]);
    fc_machine.reset();
    fc = fc_machine.next();
    EXPECT_EQ(fc, expected_sequence[0]);

    REBL::MCSSettings mcs_settings2 = {false, 1, 3, 0.0, 0.0};
    fc_machine.reset(mcs_settings2);

    std::vector<std::vector<std::string>> expected_sequence2 = {

        {"1"},
        {"1", "2"},
        {"1", "2", "3"},
        {"1", "2", "4"},
        {"1", "2", "5"},
        {"1", "3"},
        {"1", "3", "4"},
        {"1", "3", "5"},
        {"1", "4"},
        {"1", "4", "5"},
        {"1", "5"},
        {"2"},
        {"2", "3"},
        {"2", "3", "4"},
        {"2", "3", "5"},
        {"2", "4"},
        {"2", "4", "5"},
        {"2", "5"},
        {"3"},
        {"3", "4"},
        {"3", "4", "5"},
        {"3", "5"},
        {"4"},
        {"4", "5"},
        {"5"}};

    unsigned int ctr2 = 0;
    for (auto fc = fc_machine.next(); !fc.empty();
         fc      = fc_machine.next(), ++ctr2)
    {
        std::cout << "----------------" << "\n";
        std::cout << "fc #" << ctr << "\n";
        print_vector_of_strings(fc);
        std::cout << "\n";
        EXPECT_EQ(fc, expected_sequence2[ctr2]);
    }
    EXPECT_NE(ctr2, 0);
}

TEST(REBL_MCS, simplerbd_combinations)
{
    double expected_H = 2.511415522436466e-05;
    double expected_T = 7.264425076603002e-05;
    double expected_P = 1.824398989895749e-09;

    REBL::RBD rebl(std::string(TEST_DATA_DIR) + "/rbd_simple.db",
                   REBL::MCSSettings(false, 1, 2, 0.0, 0.0));

    auto system_result = rebl.run_minimal_cut_sets();
    std::cout << "H:" << system_result.H() << " 1/a \n";
    std::cout << "T:" << system_result.T() << " a\n";
    std::cout << "P:" << system_result.P() << " * 100%\n";
    // std::cout << "\n" << rebl.get_graph_adjacency_string(true, true) << "\n";
    expect_rel_near(system_result.H(), expected_H, 1e-4);
    expect_rel_near(system_result.T(), expected_T, 1e-4);
    expect_rel_near(system_result.P(), expected_P, 1e-4);
}

TEST(REBL_MCS, midsimplerbd_combinations)
{
    double expected_H = 0.1002329527430544;
    double expected_T = 0.000341986399289544;
    double expected_P = 3.4278306598756196e-05;

    REBL::RBD rebl(std::string(TEST_DATA_DIR) + "/rbd_mid_simple.db",
                   REBL::MCSSettings(false, 1, 5, 0.0, 0.0));

    auto system_result = rebl.run_minimal_cut_sets();
    std::cout << "H:" << system_result.H() << " 1/a \n";
    std::cout << "T:" << system_result.T() << " a\n";
    std::cout << "P:" << system_result.P() << " * 100%\n";
    // std::cout << "\n" << rebl.get_graph_adjacency_string(true, true) << "\n";
    expect_rel_near(system_result.H(), expected_H, 1e-2);
    expect_rel_near(system_result.T(), expected_T, 1e-2);
    expect_rel_near(system_result.P(), expected_P, 1e-2);
}

TEST(REBL_MCS, midsimplerbd_probability)
{
    double expected_H = 0.1002329527430544;
    double expected_T = 0.000341986399289544;
    double expected_P = 3.4278306598756196e-05;

    REBL::RBD rebl(std::string(TEST_DATA_DIR) + "/rbd_mid_simple.db",
                   REBL::MCSSettings(true, 0, 0, 1e-8, 1.0));

    auto system_result = rebl.run_minimal_cut_sets();
    std::cout << "H:" << system_result.H() << " 1/a \n";
    std::cout << "T:" << system_result.T() << " a\n";
    std::cout << "P:" << system_result.P() << " * 100%\n";
    // std::cout << "\n" << rebl.get_graph_adjacency_string(true, true) << "\n";
    expect_rel_near(system_result.H(), expected_H, 1e-2);
    expect_rel_near(system_result.T(), expected_T, 1e-2);
    expect_rel_near(system_result.P(), expected_P, 1e-2);
}

TEST(REBL_RBD, template_dump)
{
    auto db_path = std::string(TEST_DATA_DIR) + "/REBL_RBD_template_dump.db";
    if (std::filesystem::exists(std::filesystem::path(db_path.c_str())))
    {
        std::filesystem::remove(std::filesystem::path(db_path.c_str()));
    }
    REBL::RBD::spawn_rbd_db_template(db_path.c_str());
    EXPECT_TRUE(
        std::filesystem::exists(std::filesystem::path(db_path.c_str())));
}

TEST(SCV, random_rbd)
{
    std::string jsn;
    REBL::SCV scv;
    scv.init();
    scv.place("A");
    scv.place("B");
    scv.place("C");
    scv.fork();
    scv.place("D");
    scv.place("E");
    scv.rewind();
    scv.place("F");
    scv.place("G");
    scv.fork();
    scv.place("H");
    scv.rewind();
    scv.place("I");
    scv.join();
    scv.place("J");
    scv.join();
    jsn = scv.get_json_string();
    EXPECT_EQ(
        jsn,
        R"result(["A","B","C",["D","E"],["F","G",["H"],["I"],null,"J"],null])result");
}

TEST(SCV, random_rbd_dump_to_disk)
{
    std::string jsn;
    REBL::SCV scv;
    scv.init();
    scv.place("A");
    scv.place("B");
    scv.place("C");
    scv.fork();
    scv.place("D");
    scv.place("E");
    scv.rewind();
    scv.place("F");
    scv.place("G");
    scv.fork();
    scv.place("H");
    scv.rewind();
    scv.place("I");
    scv.join();
    scv.place("J");
    scv.join();
    jsn = scv.get_json_string();
    EXPECT_EQ(
        jsn,
        R"result(["A","B","C",["D","E"],["F","G",["H"],["I"],null,"J"],null])result");
    auto file_path = std::string(TEST_DATA_DIR) + "/scv_dump.json";
    scv.save_to_file(file_path);
    std::ifstream jF;
    jF.open(file_path);
    std::string jBuf;
    jF >> jBuf;
    EXPECT_EQ(
        jBuf,
        R"result(["A","B","C",["D","E"],["F","G",["H"],["I"],null,"J"],null])result");
}

#endif // _REBL_TEST_HPP