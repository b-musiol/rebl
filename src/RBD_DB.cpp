/**
 * Database for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include/RBD_DB.hpp"
#include "Kochs.hpp"
#include <Litesaver.hpp>
#include <format>
#include <stdexcept>

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

std::unordered_map<std::string, Kochs::Object> DB::Connection::
    get_rbd_components()
{
    std::unordered_map<std::string, Kochs::Object> rbd_components;

    // Factors for H, T and length (for units mostly)
    auto H_factor = std::get<double>(get_input_unique("rbd", "H_factor"));
    auto T_factor = std::get<double>(get_input_unique("rbd", "T_factor"));
    auto length_factor =
        std::get<double>(get_input_unique("rbd", "length_factor"));

    // Queries
    auto query_component_types = R"sql(
    SELECT "name", "H", "T" FROM "input_component_types";
    )sql";
    auto query_components      = R"sql(
    SELECT "name", "H", "T", "length", "type", "ideal"
    FROM "input_components";
    )sql";

    // Getting raw data
    auto table_component_types = direct_read_access(query_component_types);
    auto table_components      = direct_read_access(query_components);

    constexpr std::uint64_t col_name   = 0;
    constexpr std::uint64_t col_H      = 1;
    constexpr std::uint64_t col_T      = 2;
    constexpr std::uint64_t col_length = 3;
    constexpr std::uint64_t col_type   = 4;
    constexpr std::uint64_t col_ideal  = 5;

    // Component types
    std::unordered_map<std::string, Kochs::Object> component_types;
    for (auto &row : table_component_types.data)
    {
        component_types[row.get_text(col_name)] =
            Kochs::Object(row.get_real(col_H), row.get_real(col_T));
    }

    // Assembling the component data
    for (auto &row : table_components.data)
    {
        // ignore empty rows.
        // They are ugly but let's not force the user to be clean.
        if (!row.is_null(col_name))
        {
            auto component_name = row.get_text(col_name);
            // Priority #1: Is it ideal?
            if (row.is_null(col_ideal) || row.get_integer(col_ideal) != 1)
            {
                // Priority #2: Explicit H and T
                if (row.is_null(col_H) || row.is_null(col_T))
                {
                    // Priority #3: Type from the DB
                    if (!row.is_null(col_type))
                    {
                        auto component_type = row.get_text(col_type);
                        // Check if that type exists
                        if (component_types.find(component_type) !=
                            component_types.end())
                        {
                            // Type exists
                            // Does a length exist?
                            if (row.is_null(col_length))
                            {
                                // No length, flat H and T from type
                                rbd_components[component_name] = Kochs::Object(
                                    component_types.at(component_type).H() *
                                        H_factor,
                                    component_types.at(component_type).T() *
                                        T_factor);
                            }
                            else
                            {
                                // Length exists, multiply H*length with H and T
                                // from type
                                auto length =
                                    row.get_real(col_length) * length_factor;
                                rbd_components[component_name] = Kochs::Object(
                                    component_types.at(component_type).H() *
                                        H_factor * length * length_factor,
                                    component_types.at(component_type).T() *
                                        T_factor);
                            }
                        }
                        else
                        {
                            // Hard error. No type exists.
                            throw std::runtime_error(std::format(
                                "REBL RBD_DB Error: For component [{}] in "
                                "input_components: undefined type [{}] "
                                "specified "
                                "in the components input table.",
                                component_name,
                                component_type));
                        }
                    }
                    else
                    {
                        // Hard error. There is no type given despite all prior
                        // checks failing. If a component is to be ideal, it
                        // should be denoted as ideal OR not appear in the
                        // components list.
                        throw std::runtime_error(std::format(
                            "REBL RBD_DB Error: For component [{}] in "
                            "input_components: insufficient information "
                            "specified "
                            "in the components input table.",
                            component_name));
                    }
                }
                else
                {
                    // Is there a length?
                    auto H = row.get_real(col_H) * H_factor;
                    auto T = row.get_real(col_T) * T_factor;
                    if (row.is_null(col_length))
                    {
                        // No length, flat H and T
                        rbd_components[component_name] = Kochs::Object(H, T);
                    }
                    else
                    {
                        // Length exists, multiply H*length
                        auto length = row.get_real(col_length) * length_factor;
                        rbd_components[component_name] =
                            Kochs::Object(H * length, T);
                    }
                }
            }
            else /*is ideal*/
            {
                // Should be ideal
                // put an ideal object in
                rbd_components[component_name] = Kochs::Object();
            }
        }
    }

    // Done, return the data
    return rbd_components;
}