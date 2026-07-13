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
#include <string>
#include <string_view>

namespace REBL
{

namespace DB
{

class Connection : public Litesaver::Base
{
  private:
    std::string raw_rbd_json;

  public:
    Connection(std::filesystem::path db_path);
    ~Connection();

  public:
    /**
     * Gets the rbd JSON from the database.
     */
    nlohmann::json get_rbd();
    /**
     * Gets the rbd JSON from a string. This also stores it in `raw_rbd_json`.
     */
    nlohmann::json get_rbd(std::string_view rbd_json_string);
    /**
     * Gets the last rbd that has been read in as JSON-String. You must call
     * get_rbd() first, or this is not updated.
     */
    std::string get_rbd_raw_json();
    /**
     * Gets the rbd components as a map of explicit reliability data.
     */
    std::unordered_map<std::string, Kochs::Object> get_rbd_components();

  public:
    static void spawn_rbd_db_template(std::filesystem::path db_path);
};

} // namespace DB

} // namespace REBL

#endif