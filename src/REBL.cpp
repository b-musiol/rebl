/**
 * REBL - Library for handling and analysis of REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */
#include "../include/REBL.hpp"
#include "../include/Core.hpp"

using namespace REBL;

RBD::RBD(const std::string_view rbd_db_path)
    : core(std::make_unique<Core>(rbd_db_path))
{
}

RBD::~RBD()
{
}