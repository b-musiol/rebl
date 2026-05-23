/**
 * Database for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#ifndef _REBL_RBD_DB_
#define _REBL_RBD_DB_

#include <Kochs.hpp>
#include <Litesaver.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace REBL
{

namespace DB
{

class Connection : public Litesaver::Base
{

  public:
    Connection(std::filesystem::path db_path);
    ~Connection();

  public:
    /**
     * Gets the rbd JSON from the database.
     */
    nlohmann::json get_rbd();
    std::unordered_map<std::string, Kochs::Object> get_rbd_components();
};

} // namespace DB

} // namespace REBL

#endif