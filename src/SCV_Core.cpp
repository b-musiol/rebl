/**
 * Core Implementation for REBL-SCV - Library for handling and analysis of
 * REliability BLock diagrams.
 * Provides methods to build the REBL reliability block diagram as a JSON
 * through code.
 *
 * "In the rear with the gear."
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include_private/SCV_Core.hpp"

#include <fstream>
// #include <iostream>
using namespace REBL;

SCV::Core::Core() = default;

SCV::Core::~Core() = default;

nlohmann::json *SCV::Core::get_cursor()
{
    nlohmann::json *cursor = &m_json_buffer;
    for (auto ix : m_ix_stack) { cursor = &((*cursor)[ix]); }
    return cursor;
}

std::string SCV::Core::get_json_string() { return m_json_buffer.dump(); }

void SCV::Core::save_to_file(std::string json_path)
{
    std::ofstream j_f;
    j_f.open(json_path.c_str());

    j_f << get_json_string();

    j_f.close();
}

void SCV::Core::init() { m_json_buffer.clear(); }

void SCV::Core::place(const std::string_view block_name)
{
    auto json_cursor = get_cursor();
    json_cursor->push_back(block_name);
}

void SCV::Core::fork()
{
    auto json_cursor = get_cursor();
    json_cursor->push_back(nlohmann::json::array());
    m_ix_stack.push_back(json_cursor->size() - 1);
}

void SCV::Core::join()
{
    m_ix_stack.pop_back();
    auto json_cursor = get_cursor();
    json_cursor->push_back(nullptr);
}

void SCV::Core::rewind()
{
    m_ix_stack.pop_back();
    fork();
}