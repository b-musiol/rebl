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

void DB::Connection::spawn_rbd_db_template(std::filesystem::path db_path)
{
    ////Template for descriptions/help
    // constexpr const char * help_<input/output>_<table>_<col> =
    // R"_helptext_()_helptext_";

    constexpr const char *help_input_component_types_name =
        R"_helptext_(Name of the component type. This must be unique.)_helptext_";
    constexpr const char *help_input_component_types_H =
        R"_helptext_(Rate of Failure, possibly per length unit.)_helptext_";
    constexpr const char *help_input_component_types_T =
        R"_helptext_(Duration of a failed state.)_helptext_";

    constexpr const char *help_input_components_name =
        R"_helptext_(Name of the component. This must be unique. If the component does not appear here but is used in the RBD, then it is considered to be ideal.)_helptext_";
    constexpr const char *help_input_components_H =
        R"_helptext_(Rate of Failure)_helptext_";
    constexpr const char *help_input_components_T =
        R"_helptext_(Duration of a failed state.)_helptext_";
    constexpr const char *help_input_components_length =
        R"_helptext_(The length of the component. If NULL, then it is implicitly treated as 1. This is multiplied onto H (example: lines).)_helptext_";
    constexpr const char *help_input_components_type =
        R"_helptext_(The name of the type of the component, as per input_component_data. If NULL, then the explicit H and T are used, otherwise they are ignored.)_helptext_";
    constexpr const char *help_input_components_ideal =
        R"_helptext_(1 if the component is ideal, 0 if not. If the component is ideal, it is treated as if H and T are 0.)_helptext_";

    constexpr const char *help_input_rbd_rbd =
        R"_helptext_(JSON representation of the rbd.
Generally, if strings (if across the rbd the same string appears multiple times, it is a different instance of the same component, kind of like a switch that affects multiple parts of a circuit at the same time works, so if one fails, all instances must fail at the same time) appear consecutively, that means the blocks are in series. "null" is treated the same, but is a special block that is ideal and has no name and is only needed to sort things and to "glue" parallel subblocks together. Each [] defines a start and end point of the blocks inside (strings). If a [] appears at the start of a surrounding [] (nested), that means it inherits the starting point. If a [] appears after another [] (in series), that does not mean these are in series, but that their start and end points are the same. Basically that's how these two [] are set in parallel. That's why I need the special "null" block that does nothing, because that way I can do something like "(a OR b) AND (c OR d)" by saying [["a"], ["b"], null, ["c"], ["d"]]. Without the "null", I would have all a, b, c, d in parallel. Lastly, when a [] ends and then another [] ends, then these end points are merged as well. Meaning that in the above example, the end points of c and d have been merged to one by the previous definition, and now the entire subblock also merges its end point onto both the ends of c and d. With this, RBDs can be fully modeled. "null" can be added anywhere (not as a string but as a null entity, the string "null" would be a full block!) and behaves exactly like a real (string) block.)_helptext_";
    constexpr const char *help_input_rbd_H_factor =
        R"_helptext_(This is multiplied with each and every H. Useful to align units.)_helptext_";
    constexpr const char *help_input_rbd_T_factor =
        R"_helptext_(This is multiplied with each and every T. Useful to align units.)_helptext_";
    constexpr const char *help_input_rbd_length_factor =
        R"_helptext_(This is multiplied with each and every length. Useful to align units.)_helptext_";
    Litesaver::TableConfig input_config;
    using ValType                   = Litesaver::ValueType;
    input_config["component_types"] = {
        {{"name", {ValType::TEXT, help_input_component_types_name}},
         {"H", {ValType::FLOAT, help_input_component_types_H}},
         {"T", {ValType::FLOAT, help_input_component_types_T}}},
        false};
    input_config["components"] = {
        {{"name", {ValType::TEXT, help_input_components_name}},
         {"H", {ValType::FLOAT, help_input_components_H}},
         {"T", {ValType::FLOAT, help_input_components_T}},
         {"length", {ValType::FLOAT, help_input_components_length}},
         {"type", {ValType::TEXT, help_input_components_type}},
         {"ideal", {ValType::INTEGER, help_input_components_ideal}}},
        false};
    input_config["rbd"] = {
        {{"rbd", {ValType::TEXT, help_input_rbd_rbd}},
         {"H_factor", {ValType::FLOAT, help_input_rbd_H_factor}},
         {"T_factor", {ValType::FLOAT, help_input_rbd_T_factor}},
         {"length_factor", {ValType::FLOAT, help_input_rbd_length_factor}}},
        true};

    constexpr const char *help_output_result_summary_run_id =
        R"_helptext_(ID of the calculation run)_helptext_";
    constexpr const char *help_output_result_summary_ok =
        R"_helptext_(1 if the calculation was successful, otherwise 0)_helptext_";
    constexpr const char *help_output_result_summary_H =
        R"_helptext_(Mean Rate of Failure for the entire system.)_helptext_";
    constexpr const char *help_output_result_summary_T =
        R"_helptext_(Mean Duration of a failed state of the entire system.)_helptext_";
    constexpr const char *help_output_result_summary_use_probability =
        R"_helptext_(1 if probability bounds for failure combinations were used, 0 if the combination size was used.)_helptext_";
    constexpr const char *help_output_result_summary_min_combination_size =
        R"_helptext_(minimum combination size in this run)_helptext_";
    constexpr const char *help_output_result_summary_max_combination_size =
        R"_helptext_(maximum combination size in this run)_helptext_";
    constexpr const char *help_output_result_summary_min_probability =
        R"_helptext_(minimum combination probability in this run)_helptext_";
    constexpr const char *help_output_result_summary_max_probability =
        R"_helptext_(maximum combination probability in this run)_helptext_";

    constexpr const char *help_output_result_fc_run_id =
        R"_helptext_(ID of the calculation run)_helptext_";
    constexpr const char *help_output_result_fc_fc_id =
        R"_helptext_(ID of the failure combination, only valid within a run.)_helptext_";
    constexpr const char *help_output_result_fc_H =
        R"_helptext_(Rate of Failure for the failure combination)_helptext_";
    constexpr const char *help_output_result_fc_T =
        R"_helptext_(Duration of a failed state for the failure combination)_helptext_";

    constexpr const char *help_output_detail_fc_run_id =
        R"_helptext_(ID of the calculation run)_helptext_";
    constexpr const char *help_output_detail_fc_fc_id =
        R"_helptext_(ID of the failure combination, only valid within a run.)_helptext_";
    constexpr const char *help_output_detail_fc_component =
        R"_helptext_(component name)_helptext_";
    constexpr const char *help_output_detail_fc_H =
        R"_helptext_(Rate of Failure for the component)_helptext_";
    constexpr const char *help_output_detail_fc_T =
        R"_helptext_(Duration of a failed state for the component)_helptext_";
    Litesaver::TableConfig output_config;
    output_config["result_summary"] = {
        {{"run_id", {ValType::INTEGER, help_output_result_summary_run_id}},
         {"ok", {ValType::FLOAT, help_output_result_summary_ok}},
         {"H", {ValType::FLOAT, help_output_result_summary_H}},
         {"T", {ValType::FLOAT, help_output_result_summary_T}},
         {"use_probability",
          {ValType::INTEGER, help_output_result_summary_use_probability}},
         {"min_combination_size",
          {ValType::INTEGER, help_output_result_summary_min_combination_size}},
         {"max_combination_size",
          {ValType::INTEGER, help_output_result_summary_max_combination_size}},
         {"min_probability",
          {ValType::FLOAT, help_output_result_summary_min_probability}},
         {"max_probability",
          {ValType::FLOAT, help_output_result_summary_max_probability}}},
        false};
    output_config["result_fc"] = {
        {
            {"run_id", {ValType::INTEGER, help_output_result_fc_run_id}},
            {"fc_id", {ValType::INTEGER, help_output_result_fc_fc_id}},
            {"H", {ValType::FLOAT, help_output_result_fc_H}},
            {"T", {ValType::FLOAT, help_output_result_fc_T}},
        },
        false};
    output_config["detail_fc"] = {
        {
            {"run_id", {ValType::INTEGER, help_output_detail_fc_run_id}},
            {"fc_id", {ValType::INTEGER, help_output_detail_fc_fc_id}},
            {"component", {ValType::TEXT, help_output_detail_fc_component}},
            {"H", {ValType::FLOAT, help_output_detail_fc_H}},
            {"T", {ValType::FLOAT, help_output_detail_fc_T}},
        },
        false};

    Litesaver::Base template_db(db_path, input_config, output_config);
}