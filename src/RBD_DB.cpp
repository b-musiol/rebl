/**
 * Database for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/RBD_DB.hpp"
#include <Litesaver.hpp>

using namespace REBL;

DB::Connection::Connection(std::filesystem::path db_path)
    : Litesaver::Base(db_path)
{
}

DB::Connection::~Connection()
{
}
nlohmann::json DB::Connection::get_rbd()
{
    auto raw_rbd_json = std::get<std::string>(get_input_unique("rbd", "rbd"));
    auto rbd_json     = nlohmann::json::parse(raw_rbd_json);
    return rbd_json;
}