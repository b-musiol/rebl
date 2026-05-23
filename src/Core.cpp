/**
 * Core Implementation for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/Core.hpp"
#include <memory>

using namespace REBL;

RBD::Core::Core(const std::string_view rbd_db_path)
    : rbd(std::make_unique<
          KnoKan::DirectedGraph<std::string, ComponentData, EmptyP>>()),
      rbd_db_path(rbd_db_path), rbd_db(rbd_db_path)
{
}

RBD::Core::~Core()
{
}