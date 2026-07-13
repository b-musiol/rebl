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

constexpr const char *update_output_result_summary = R"sql(
UPDATE "output_result_summary"
SET 
    "ok" = ?,
    "H" = ?,
    "T" = ?
WHERE
    "run_id" = ?;
)sql";
} // namespace Query
} // namespace DB
} // namespace REBL

#endif // _REBL_RBD_DB_QUERIES_HPP