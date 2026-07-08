/**
 * Implementation for REBL-SCV - Library for handling and analysis of
 * REliability BLock diagrams.
 * Provides methods to build the REBL reliability block diagram as a JSON
 * through code.
 *
 * "Reportin' for duty."
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/SCV.hpp"
#include "../include_private/SCV_Core.hpp"

using namespace REBL;

SCV::SCV() : m_core(std::make_unique<SCV::Core>())
{
}

SCV::~SCV()
{
}

std::string SCV::get_json_string()
{
    return m_core->get_json_string();
}

void SCV::save_to_file(std::string json_path)
{
    m_core->save_to_file(json_path);
}

void SCV::init()
{
    m_core->init();
}

void SCV::place(const std::string_view block_name)
{
    m_core->place(block_name);
}

void SCV::fork()
{
    m_core->fork();
}

void SCV::join()
{
    m_core->join();
}

void SCV::rewind()
{
    m_core->rewind();
}