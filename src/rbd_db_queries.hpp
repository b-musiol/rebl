#ifndef _REBL_RBD_DB_QUERIES_HPP
#define _REBL_RBD_DB_QUERIES_HPP

namespace REBL
{

namespace DB
{

namespace Query
{

constexpr const char *select_largest_run_id = R"sql(
SELECT
    MAX("run_id")
FROM "output_result_summary";
)sql";

constexpr const char *select_all_component_names = R"sql(
SELECT name FROM input_components;
)sql";

constexpr const char *select_all_component_type_names = R"sql(
SELECT name FROM input_component_types;;
)sql";

constexpr const char *insert_into_input_component_types = R"sql(
INSERT INTO "input_component_types"("name","H","T")
VALUES (?,?,?);
)sql";

constexpr const char *insert_into_input_components = R"sql(
INSERT INTO "input_components"("name","H","T","length","type","ideal")
VALUES (?,?,?,?,?,?);
)sql";

constexpr const char *delete_from_input_component_types = R"sql(
DELETE FROM input_component_types WHERE name = ?;
)sql";

constexpr const char *delete_from_input_components = R"sql(
DELETE FROM input_components WHERE name = ?;
)sql";

constexpr const char *insert_into_output_detail_fc = R"sql(
INSERT INTO "output_detail_fc"
(
    "fc_id",
    "run_id",
    "H",
    "T",
    "component"
)

VALUES
(?,?,?,?,?);
)sql";

constexpr const char *insert_into_output_result_fc = R"sql(
INSERT INTO "output_result_fc"
(
    "fc_id",
    "run_id",
    "H",
    "T"
)
VALUES
(?,?,?,?);
)sql";

constexpr const char *insert_into_output_result_summary = R"sql(
INSERT INTO "output_result_summary"
(
    "run_id",
    "rbd",
    "ok",
    "H",
    "T",
    "use_probability",
    "min_combination_size",
    "max_combination_size",
    "min_probability",
    "max_probability"
)
VALUES
(?,?,?,?,?,?,?,?,?,?);
)sql";

constexpr int run_id_output_detail_fc_ix          = 0;
constexpr const char *select_all_output_detail_fc = R"sql(
SELECT
    "fc_id",
    "run_id",
    "H",
    "T",
    "component"
FROM "output_detail_fc";
)sql";

constexpr int run_id_output_result_fc_ix          = 1;
constexpr const char *select_all_output_result_fc = R"sql(
SELECT
    "fc_id",
    "run_id",
    "H",
    "T"
FROM "output_result_fc";
)sql";

constexpr int run_id_output_result_summary_ix          = 0;
constexpr const char *select_all_output_result_summary = R"sql(
SELECT
    "run_id",
    "rbd",
    "ok",
    "H",
    "T",
    "use_probability",
    "min_combination_size",
    "max_combination_size",
    "min_probability",
    "max_probability"
FROM "output_result_summary";
)sql";

constexpr const char *update_output_result_summary = R"sql(
UPDATE "output_result_summary"
SET 
    "ok" = ?,
    "H" = ?,
    "T" = ?
WHERE
    "run_id" = ?;
)sql";

constexpr const char *update_input_rbd_real = R"sql(
UPDATE "input_rbd"
SET "valFloat" = ?
WHERE "key" = ?;
)sql";

} // namespace Query
} // namespace DB
} // namespace REBL

#endif // _REBL_RBD_DB_QUERIES_HPP