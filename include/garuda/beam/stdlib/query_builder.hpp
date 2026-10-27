#pragma once
// Garuda Standard Library: SQL Query Builder & Parameter Binding (`query_builder.hpp`)
#include "sqlite3.h"
#include "../core/term.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace garuda::beam::stdlib {

struct SqlParam {
    enum class Type { Text, Int64, Double, Null } type = Type::Text;
    std::string text_val;
    std::int64_t int_val = 0;
    double double_val = 0.0;

    static SqlParam make_text(std::string s) {
        SqlParam p;
        p.type = Type::Text;
        p.text_val = std::move(s);
        return p;
    }

    static SqlParam make_int(std::int64_t i) {
        SqlParam p;
        p.type = Type::Int64;
        p.int_val = i;
        return p;
    }

    static SqlParam make_double(double d) {
        SqlParam p;
        p.type = Type::Double;
        p.double_val = d;
        return p;
    }
};

struct SqlQuery {
    std::string sql;
    std::vector<SqlParam> params;
};

class QueryBuilder {
public:
    enum class Mode { Select, Insert, Update, Delete };

    QueryBuilder() = default;

    static QueryBuilder create() {
        return QueryBuilder();
    }

    QueryBuilder& select(const std::vector<std::string>& columns = {"*"}) {
        mode_ = Mode::Select;
        columns_ = columns;
        return *this;
    }

    QueryBuilder& from(const std::string& table) {
        table_ = table;
        return *this;
    }

    static QueryBuilder select_from(const std::string& table, const std::vector<std::string>& columns = {"*"}) {
        QueryBuilder qb;
        qb.mode_ = Mode::Select;
        qb.table_ = table;
        qb.columns_ = columns;
        return qb;
    }

    QueryBuilder& insert_into(const std::string& table, const std::map<std::string, std::string>& data) {
        mode_ = Mode::Insert;
        table_ = table;
        columns_.clear();
        params_.clear();
        for (const auto& [k, v] : data) {
            columns_.push_back(k);
            params_.push_back(SqlParam::make_text(v));
        }
        return *this;
    }

    static QueryBuilder insert_into(const std::string& table, const std::vector<std::string>& columns, const std::vector<SqlParam>& values) {
        QueryBuilder qb;
        qb.mode_ = Mode::Insert;
        qb.table_ = table;
        qb.columns_ = columns;
        qb.params_ = values;
        return qb;
    }

    QueryBuilder& update(const std::string& table) {
        mode_ = Mode::Update;
        table_ = table;
        return *this;
    }

    QueryBuilder& set(const std::string& column, const std::string& value) {
        set_clauses_.push_back(column + " = ?");
        params_.push_back(SqlParam::make_text(value));
        return *this;
    }

    QueryBuilder& set(const std::map<std::string, std::string>& data) {
        for (const auto& [k, v] : data) {
            set(k, v);
        }
        return *this;
    }

    QueryBuilder& delete_from(const std::string& table) {
        mode_ = Mode::Delete;
        table_ = table;
        return *this;
    }

    QueryBuilder& where(const std::string& column, const std::string& op, const SqlParam& param) {
        where_clauses_.push_back(column + " " + op + " ?");
        params_.push_back(param);
        return *this;
    }

    QueryBuilder& where(const std::string& column, const std::string& op, const std::string& value) {
        return where(column, op, SqlParam::make_text(value));
    }

    QueryBuilder& where_text(const std::string& column, const std::string& op, const std::string& text) {
        return where(column, op, SqlParam::make_text(text));
    }

    QueryBuilder& where_int(const std::string& column, const std::string& op, std::int64_t val) {
        return where(column, op, SqlParam::make_int(val));
    }

    SqlQuery to_sql() const {
        SqlQuery result;
        std::ostringstream oss;

        if (mode_ == Mode::Select) {
            oss << "SELECT ";
            if (columns_.empty()) {
                oss << "*";
            } else {
                for (std::size_t i = 0; i < columns_.size(); ++i) {
                    if (i > 0) oss << ", ";
                    oss << columns_[i];
                }
            }
            oss << " FROM " << table_;

            if (!where_clauses_.empty()) {
                oss << " WHERE ";
                for (std::size_t i = 0; i < where_clauses_.size(); ++i) {
                    if (i > 0) oss << " AND ";
                    oss << where_clauses_[i];
                }
            }
        } else if (mode_ == Mode::Insert) {
            oss << "INSERT INTO " << table_ << " (";
            for (std::size_t i = 0; i < columns_.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << columns_[i];
            }
            oss << ") VALUES (";
            for (std::size_t i = 0; i < columns_.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << "?";
            }
            oss << ")";
        } else if (mode_ == Mode::Update) {
            oss << "UPDATE " << table_ << " SET ";
            for (std::size_t i = 0; i < set_clauses_.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << set_clauses_[i];
            }
            if (!where_clauses_.empty()) {
                oss << " WHERE ";
                for (std::size_t i = 0; i < where_clauses_.size(); ++i) {
                    if (i > 0) oss << " AND ";
                    oss << where_clauses_[i];
                }
            }
        } else if (mode_ == Mode::Delete) {
            oss << "DELETE FROM " << table_;
            if (!where_clauses_.empty()) {
                oss << " WHERE ";
                for (std::size_t i = 0; i < where_clauses_.size(); ++i) {
                    if (i > 0) oss << " AND ";
                    oss << where_clauses_[i];
                }
            }
        }

        result.sql = oss.str();
        result.params = params_;
        return result;
    }

    std::pair<std::string, std::vector<std::string>> build() const {
        SqlQuery sq = to_sql();
        std::vector<std::string> str_params;
        str_params.reserve(sq.params.size());
        for (const auto& p : sq.params) {
            if (p.type == SqlParam::Type::Text) {
                str_params.push_back(p.text_val);
            } else if (p.type == SqlParam::Type::Int64) {
                str_params.push_back(std::to_string(p.int_val));
            } else if (p.type == SqlParam::Type::Double) {
                str_params.push_back(std::to_string(p.double_val));
            } else {
                str_params.push_back("");
            }
        }
        return {sq.sql, str_params};
    }

    static bool execute_prepared(sqlite3* db, const SqlQuery& query) {
        if (!db) return false;
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, query.sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK || !stmt) return false;

        for (std::size_t i = 0; i < query.params.size(); ++i) {
            int idx = static_cast<int>(i + 1);
            const auto& p = query.params[i];
            if (p.type == SqlParam::Type::Text) {
                sqlite3_bind_text(stmt, idx, p.text_val.c_str(), -1, SQLITE_TRANSIENT);
            } else if (p.type == SqlParam::Type::Int64) {
                sqlite3_bind_int64(stmt, idx, p.int_val);
            } else if (p.type == SqlParam::Type::Double) {
                sqlite3_bind_double(stmt, idx, p.double_val);
            } else {
                sqlite3_bind_null(stmt, idx);
            }
        }

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return (rc == SQLITE_DONE || rc == SQLITE_ROW);
    }

    static std::vector<std::vector<std::string>> fetch_all_prepared(sqlite3* db, const SqlQuery& query) {
        std::vector<std::vector<std::string>> rows;
        if (!db) return rows;

        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db, query.sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK || !stmt) return rows;

        for (std::size_t i = 0; i < query.params.size(); ++i) {
            int idx = static_cast<int>(i + 1);
            const auto& p = query.params[i];
            if (p.type == SqlParam::Type::Text) {
                sqlite3_bind_text(stmt, idx, p.text_val.c_str(), -1, SQLITE_TRANSIENT);
            } else if (p.type == SqlParam::Type::Int64) {
                sqlite3_bind_int64(stmt, idx, p.int_val);
            } else if (p.type == SqlParam::Type::Double) {
                sqlite3_bind_double(stmt, idx, p.double_val);
            } else {
                sqlite3_bind_null(stmt, idx);
            }
        }

        int col_count = sqlite3_column_count(stmt);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::vector<std::string> row;
            row.reserve(col_count);
            for (int col = 0; col < col_count; ++col) {
                const unsigned char* txt = sqlite3_column_text(stmt, col);
                row.push_back(txt ? reinterpret_cast<const char*>(txt) : "");
            }
            rows.push_back(std::move(row));
        }

        sqlite3_finalize(stmt);
        return rows;
    }

private:
    Mode mode_ = Mode::Select;
    std::string table_;
    std::vector<std::string> columns_;
    std::vector<std::string> set_clauses_;
    std::vector<std::string> where_clauses_;
    std::vector<SqlParam> params_;
};

} // namespace garuda::beam::stdlib
