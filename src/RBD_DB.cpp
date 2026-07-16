/**
 * Database for REBL - Library for handling and analysis of
 * REliability BLock diagrams.
 *
 * Author: Bartek Musiol (bmusiol@proton.me)
 *
 * See LICENSE
 */

#include "../include_private/RBD_DB.hpp"
#include "Kochs.hpp"
#include "SQLiteDB.hpp"
#include "rbd_db_queries.hpp"
#include <Litesaver.hpp>
#include <algorithm>
#include <format>
#include <optional>
#include <stdexcept>

using namespace REBL;

DB::Connection::Connection(std::filesystem::path db_path)
    : Litesaver::Base(db_path)
{
}

DB::Connection::~Connection() = default;
nlohmann::json DB::Connection::get_rbd()
{
    raw_rbd_json  = std::get<std::string>(get_input_unique("rbd", "rbd"));
    auto rbd_json = nlohmann::json::parse(raw_rbd_json);
    return rbd_json;
}
nlohmann::json DB::Connection::get_rbd(std::string_view rbd_json_string)
{
    raw_rbd_json  = rbd_json_string;
    auto rbd_json = nlohmann::json::parse(raw_rbd_json);
    return rbd_json;
}

std::string DB::Connection::get_rbd_raw_json() { return raw_rbd_json; }

std::unordered_map<std::string, Kochs::Object> DB::Connection::
    get_rbd_components()
{
    std::unordered_map<std::string, Kochs::Object> rbd_components;

    // Factors for H, T and length (for units mostly)
    auto h_factor = std::get<double>(get_input_unique("rbd", "H_factor"));
    auto t_factor = std::get<double>(get_input_unique("rbd", "T_factor"));
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
    constexpr std::uint64_t col_h      = 1;
    constexpr std::uint64_t col_t      = 2;
    constexpr std::uint64_t col_length = 3;
    constexpr std::uint64_t col_type   = 4;
    constexpr std::uint64_t col_ideal  = 5;

    // Component types
    std::unordered_map<std::string, Kochs::Object> component_types;
    for (auto &row : table_component_types.data)
    {
        component_types[row.get_text(col_name)] =
            Kochs::Object(row.get_real(col_h), row.get_real(col_t));
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
                if (row.is_null(col_h) || row.is_null(col_t))
                {
                    // Priority #3: Type from the DB
                    if (!row.is_null(col_type))
                    {
                        auto component_type = row.get_text(col_type);
                        // Check if that type exists
                        if (component_types.contains(component_type))
                        {
                            // Type exists
                            // Does a length exist?
                            if (row.is_null(col_length))
                            {
                                // No length, flat H and T from type
                                rbd_components[component_name] = Kochs::Object(
                                    component_types.at(component_type).H() *
                                        h_factor,
                                    component_types.at(component_type).T() *
                                        t_factor);
                            }
                            else
                            {
                                // Length exists, multiply H*length with H and T
                                // from type
                                auto length =
                                    row.get_real(col_length) * length_factor;
                                rbd_components[component_name] = Kochs::Object(
                                    component_types.at(component_type).H() *
                                        h_factor * length * length_factor,
                                    component_types.at(component_type).T() *
                                        t_factor);
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
                    auto h = row.get_real(col_h) * h_factor;
                    auto t = row.get_real(col_t) * t_factor;
                    if (row.is_null(col_length))
                    {
                        // No length, flat H and T
                        rbd_components[component_name] = Kochs::Object(h, t);
                    }
                    else
                    {
                        // Length exists, multiply H*length
                        auto length = row.get_real(col_length) * length_factor;
                        rbd_components[component_name] =
                            Kochs::Object(h * length, t);
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
        R"_helptext_(Name of the component type. This must be unique. Multiply with H_factor to neutralize implicit units.)_helptext_";
    constexpr const char *help_input_component_types_h =
        R"_helptext_(Rate of Failure, possibly per length unit. Multiply with T_factor to neutralize implicit units.)_helptext_";
    constexpr const char *help_input_component_types_t =
        R"_helptext_(Duration of a failed state.)_helptext_";

    constexpr const char *help_input_components_name =
        R"_helptext_(Name of the component. This must be unique. If the component does not appear here but is used in the RBD, then it is considered to be ideal.)_helptext_";
    constexpr const char *help_input_components_h =
        R"_helptext_(Rate of Failure. Multiply with H_factor to neutralize implicit units.)_helptext_";
    constexpr const char *help_input_components_t =
        R"_helptext_(Duration of a failed state. Multiply with T_factor to neutralize implicit units.)_helptext_";
    constexpr const char *help_input_components_length =
        R"_helptext_(The length of the component. If NULL, then it is implicitly treated as 1. This is multiplied onto H (example: lines).)_helptext_";
    constexpr const char *help_input_components_type =
        R"_helptext_(The name of the type of the component, as per input_component_data. If NULL, then the explicit H and T are used, otherwise they are ignored.)_helptext_";
    constexpr const char *help_input_components_ideal =
        R"_helptext_(1 if the component is ideal, 0 if not. If the component is ideal, it is treated as if H and T are 0.)_helptext_";

    constexpr const char *help_input_rbd_rbd =
        R"_helptext_(JSON representation of the rbd.
Generally, if strings (if across the rbd the same string appears multiple times, it is a different instance of the same component, kind of like a switch that affects multiple parts of a circuit at the same time works, so if one fails, all instances must fail at the same time) appear consecutively, that means the blocks are in series. "null" is treated the same, but is a special block that is ideal and has no name and is only needed to sort things and to "glue" parallel subblocks together. Each [] defines a start and end point of the blocks inside (strings). If a [] appears at the start of a surrounding [] (nested), that means it inherits the starting point. If a [] appears after another [] (in series), that does not mean these are in series, but that their start and end points are the same. Basically that's how these two [] are set in parallel. That's why I need the special "null" block that does nothing, because that way I can do something like "(a OR b) AND (c OR d)" by saying [["a"], ["b"], null, ["c"], ["d"]]. Without the "null", I would have all a, b, c, d in parallel. Lastly, when a [] ends and then another [] ends, then these end points are merged as well. Meaning that in the above example, the end points of c and d have been merged to one by the previous definition, and now the entire subblock also merges its end point onto both the ends of c and d. With this, RBDs can be fully modeled. "null" can be added anywhere (not as a string but as a null entity, the string "null" would be a full block!) and behaves exactly like a real (string) block.)_helptext_";
    constexpr const char *help_input_rbd_h_factor =
        R"_helptext_(This is multiplied with each and every H. Useful to align units.)_helptext_";
    constexpr const char *help_input_rbd_t_factor =
        R"_helptext_(This is multiplied with each and every T. Useful to align units.)_helptext_";
    constexpr const char *help_input_rbd_length_factor =
        R"_helptext_(This is multiplied with each and every length. Useful to align units.)_helptext_";
    Litesaver::TableConfig input_config;
    using ValType                   = Litesaver::ValueType;
    input_config["component_types"] = {
        .content   = {{"name",
                       {.value_type  = ValType::TEXT,
                        .description = help_input_component_types_name}},
                      {"H",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_component_types_h}},
                      {"T",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_component_types_t}}},
        .is_unique = false};
    input_config["components"] = {
        .content   = {{"name",
                       {.value_type  = ValType::TEXT,
                        .description = help_input_components_name}},
                      {"H",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_components_h}},
                      {"T",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_components_t}},
                      {"length",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_components_length}},
                      {"type",
                       {.value_type  = ValType::TEXT,
                        .description = help_input_components_type}},
                      {"ideal",
                       {.value_type  = ValType::INTEGER,
                        .description = help_input_components_ideal}}},
        .is_unique = false};
    input_config["rbd"] = {
        .content   = {{"rbd",
                       {.value_type  = ValType::TEXT,
                        .description = help_input_rbd_rbd}},
                      {"H_factor",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_rbd_h_factor}},
                      {"T_factor",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_rbd_t_factor}},
                      {"length_factor",
                       {.value_type  = ValType::FLOAT,
                        .description = help_input_rbd_length_factor}}},
        .is_unique = true};

    constexpr const char *help_output_result_summary_run_id =
        R"_helptext_(ID of the calculation run)_helptext_";
    constexpr const char *help_output_result_summary_ok =
        R"_helptext_(1 if the calculation was successful, otherwise 0)_helptext_";
    constexpr const char *help_output_result_summary_h =
        R"_helptext_(Mean Rate of Failure for the entire system. Multiply with H_factor to neutralize implicit units.)_helptext_";
    constexpr const char *help_output_result_summary_t =
        R"_helptext_(Mean Duration of a failed state of the entire system. Multiply with T_factor to neutralize implicit units.)_helptext_";
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
    constexpr const char *help_output_result_summary_rbd =
        R"_helptext_(JSON representation of the RBD calculated)_helptext_";
    constexpr const char *help_output_result_fc_fc_id =
        R"_helptext_(ID of the failure combination, only valid within a run.)_helptext_";
    constexpr const char *help_output_result_fc_h =
        R"_helptext_(Rate of Failure for the failure combination. Multiply with H_factor to neutralize implicit units.)_helptext_";
    constexpr const char *help_output_result_fc_t =
        R"_helptext_(Duration of a failed state for the failure combination. Multiply with T_factor to neutralize implicit units.)_helptext_";

    constexpr const char *help_output_detail_fc_run_id =
        R"_helptext_(ID of the calculation run)_helptext_";
    constexpr const char *help_output_detail_fc_fc_id =
        R"_helptext_(ID of the failure combination, only valid within a run.)_helptext_";
    constexpr const char *help_output_detail_fc_component =
        R"_helptext_(component name)_helptext_";
    constexpr const char *help_output_detail_fc_h =
        R"_helptext_(Rate of Failure for the component. Multiply with H_factor to neutralize implicit units.)_helptext_";
    constexpr const char *help_output_detail_fc_t =
        R"_helptext_(Duration of a failed state for the component. Multiply with T_factor to neutralize implicit units.)_helptext_";
    Litesaver::TableConfig output_config;
    output_config["result_summary"] = {
        .content =
            {{"run_id",
              {.value_type  = ValType::INTEGER,
               .description = help_output_result_summary_run_id}},
             {"rbd",
              {.value_type  = ValType::FLOAT,
               .description = help_output_result_summary_rbd}},
             {"ok",
              {.value_type  = ValType::FLOAT,
               .description = help_output_result_summary_ok}},
             {"H",
              {.value_type  = ValType::FLOAT,
               .description = help_output_result_summary_h}},
             {"T",
              {.value_type  = ValType::FLOAT,
               .description = help_output_result_summary_t}},
             {"use_probability",
              {.value_type  = ValType::INTEGER,
               .description = help_output_result_summary_use_probability}},
             {"min_combination_size",
              {.value_type  = ValType::INTEGER,
               .description = help_output_result_summary_min_combination_size}},
             {"max_combination_size",
              {.value_type  = ValType::INTEGER,
               .description = help_output_result_summary_max_combination_size}},
             {"min_probability",
              {.value_type  = ValType::FLOAT,
               .description = help_output_result_summary_min_probability}},
             {"max_probability",
              {.value_type  = ValType::FLOAT,
               .description = help_output_result_summary_max_probability}}},
        .is_unique = false};
    output_config["result_fc"] = {
        .content =
            {
                {"run_id",
                 {.value_type  = ValType::INTEGER,
                  .description = help_output_result_fc_run_id}},
                {"fc_id",
                 {.value_type  = ValType::INTEGER,
                  .description = help_output_result_fc_fc_id}},
                {"H",
                 {.value_type  = ValType::FLOAT,
                  .description = help_output_result_fc_h}},
                {"T",
                 {.value_type  = ValType::FLOAT,
                  .description = help_output_result_fc_t}},
            },
        .is_unique = false};
    output_config["detail_fc"] = {
        .content =
            {
                {"run_id",
                 {.value_type  = ValType::INTEGER,
                  .description = help_output_detail_fc_run_id}},
                {"fc_id",
                 {.value_type  = ValType::INTEGER,
                  .description = help_output_detail_fc_fc_id}},
                {"component",
                 {.value_type  = ValType::TEXT,
                  .description = help_output_detail_fc_component}},
                {"H",
                 {.value_type  = ValType::FLOAT,
                  .description = help_output_detail_fc_h}},
                {"T",
                 {.value_type  = ValType::FLOAT,
                  .description = help_output_detail_fc_t}},
            },
        .is_unique = false};

    Litesaver::Base template_db(db_path, input_config, output_config);
}

size_t DB::Connection::get_next_free_run_id()
{
    auto run_id_raw = direct_read_access(DB::Query::select_largest_run_id);
    // Only accept positive run ids starting with 1
    if (run_id_raw.data.size() > 0 && run_id_raw.data.front().is_integer(0))
    {
        return std::max(run_id_raw.data.front().get_integer(0) + 1,
                        static_cast<std::int64_t>(1));
    }
    else
    {
        return 1;
    }
}

std::vector<SQLiteDB::Row> DB::Connection::get_output_detail_fc_runid_offset(
    long long offset)
{
    auto raw_data = direct_read_access(DB::Query::select_all_output_detail_fc);
    for (auto &row : raw_data.data)
    {
        row.set_integer(DB::Query::run_id_output_detail_fc_ix,
                        row.get_integer(DB::Query::run_id_output_detail_fc_ix) +
                            offset);
    }
    return raw_data.data;
}

std::vector<SQLiteDB::Row> DB::Connection::get_output_result_fc_runid_offset(
    long long offset)
{
    auto raw_data = direct_read_access(DB::Query::select_all_output_result_fc);
    for (auto &row : raw_data.data)
    {
        row.set_integer(DB::Query::run_id_output_result_fc_ix,
                        row.get_integer(DB::Query::run_id_output_result_fc_ix) +
                            offset);
    }
    return raw_data.data;
}

std::vector<SQLiteDB::Row> DB::Connection::
    get_output_result_summary_runid_offset(long long offset)
{
    auto raw_data =
        direct_read_access(DB::Query::select_all_output_result_summary);
    for (auto &row : raw_data.data)
    {
        row.set_integer(
            DB::Query::run_id_output_result_summary_ix,
            row.get_integer(DB::Query::run_id_output_result_summary_ix) +
                offset);
    }
    return raw_data.data;
}

void DB::Connection::insert_output_data(
    const std::vector<SQLiteDB::Row> &output_detail_fc,
    const std::vector<SQLiteDB::Row> &output_result_fc,
    const std::vector<SQLiteDB::Row> &output_result_summary)
{
    direct_write_access(DB::Query::insert_into_output_detail_fc,
                        output_detail_fc);
    direct_write_access(DB::Query::insert_into_output_result_fc,
                        output_result_fc);
    direct_write_access(DB::Query::insert_into_output_result_summary,
                        output_result_summary);
}

std::vector<std::string> DB::Connection::get_all_component_names()
{
    std::vector<std::string> component_names;

    // Just issuing the query and getting grabbing the data
    auto raw_data = direct_read_access(DB::Query::select_all_component_names);
    component_names.reserve(raw_data.data.size());
    for (auto &rows : raw_data.data)
    {
        component_names.push_back(rows.get_text(0));
    }

    return component_names;
}

void DB::Connection::add_component_raw(
    std::string_view name,
    std::optional<double> h,
    std::optional<double> t,
    std::optional<double> length,
    std::optional<std::string_view> type_name,
    std::optional<bool> ideal)
{
    // Negative values are truncated to 0, making an ideal component. Don't use
    // negative values. They do not make sense anyway.
    if (h.has_value() && h < 0.0) h = 0.0;
    if (t.has_value() && t < 0.0) t = 0.0;

    SQLiteDB::Row params;
    params.push_text(name);

    if (h.has_value()) params.push_real(h.value());
    else params.push_null();

    if (t.has_value()) params.push_real(t.value());
    else params.push_null();

    if (length.has_value()) params.push_real(length.value());
    else params.push_null();

    if (type_name.has_value()) params.push_text(type_name.value());
    else params.push_null();

    if (ideal.has_value()) params.push_integer(ideal.value() ? 1 : 0);
    else params.push_null();

    direct_write_access(DB::Query::insert_into_input_components, params);
}

//////
// Forwarders for add_component below
//////
void DB::Connection::add_component(std::string_view name,
                                   const ComponentDataStruct::HT &data)
{ add_component(name, data.H, data.T); }

void DB::Connection::add_component(std::string_view name,
                                   const ComponentDataStruct::HTLength &data)
{ add_component(name, data.H, data.T, data.length); }

void DB::Connection::add_component(std::string_view name,
                                   ComponentDataStruct::Ideal data)
{ add_component(name); }

//////
// Forwarders for add_component above
//////

void DB::Connection::add_component(std::string_view name, double h, double t)
{
    // Version: H and T
    add_component_raw(name, h, t, std::nullopt, std::nullopt, std::nullopt);
}

void DB::Connection::add_component(std::string_view name,
                                   double h,
                                   double t,
                                   double length)
{
    // Version: H and length and T
    add_component_raw(name, h, t, length, std::nullopt, std::nullopt);
}

void DB::Connection::add_component(std::string_view name)
{
    // Version: Ideal component (not strictly necessary to set, as the default
    // is ideal, but sometimes explicitness is better.
    add_component_raw(name,
                      std::nullopt,
                      std::nullopt,
                      std::nullopt,
                      std::nullopt,
                      true);
}

void DB::Connection::add_component(std::string_view name,
                                   std::string_view type_name)
{
    // Version: Type name
    // Make sure the component type exists or the behaviour is undefined
    add_component_raw(name,
                      std::nullopt,
                      std::nullopt,
                      std::nullopt,
                      type_name,
                      std::nullopt);
}

void DB::Connection::remove_component_nofail(std::string_view name)
{
    SQLiteDB::Row params;
    params.push_text(name);
    direct_write_access(DB::Query::delete_from_input_components, params);
}

std::vector<std::string> DB::Connection::get_all_component_type_names()
{
    std::vector<std::string> component_type_names;

    // Just issuing the query and getting grabbing the data
    auto raw_data =
        direct_read_access(DB::Query::select_all_component_type_names);
    component_type_names.reserve(raw_data.data.size());
    for (auto &rows : raw_data.data)
    {
        component_type_names.push_back(rows.get_text(0));
    }

    return component_type_names;
}

void DB::Connection::add_component_type(std::string_view name,
                                        const ComponentDataStruct::HT &data)
{ add_component_type(name, data.H, data.T); }

void DB::Connection::add_component_type(std::string_view name,
                                        double h,
                                        double t)
{
    // Negative values are truncated to 0, making an ideal component. Don't use
    // negative values. They do not make sense anyway.
    h = std::max(h, 0.0);
    t = std::max(t, 0.0);

    SQLiteDB::Row params;
    params.push_text(name);
    params.push_real(h);
    params.push_real(t);

    direct_write_access(DB::Query::insert_into_input_component_types, params);
}

void DB::Connection::remove_component_type_nofail(std::string_view name)
{
    SQLiteDB::Row params;
    params.push_text(name);
    direct_write_access(DB::Query::delete_from_input_component_types, params);
}
