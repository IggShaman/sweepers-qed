#include "benchmark_stats.hpp"

#include "build_info.hpp"
#include "logger.hpp"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace b
{
template <typename Sample>
std::expected<void, std::string> log_samples(
  const std::string& dsn,
  const std::vector<Sample>& samples,
  const QString& create_table_sql,
  const QString& insert_sql,
  std::function<bool(QSqlQuery&, const Sample&)> binder)
{
    {
        auto dbh = QSqlDatabase::addDatabase("QSQLITE", "conn");
        dbh.setDatabaseName(QString::fromUtf8(dsn));
        dbh.setConnectOptions("QSQLITE_BUSY_TIMEOUT=5000");
        if (!dbh.open())
        {
            return std::unexpected{dbh.lastError().text().toStdString()};
        }

        QSqlQuery q(dbh);
        q.exec("PRAGMA journal_mode=WAL");
        q.exec("PRAGMA synchronous=FULL");
        q.exec("PRAGMA temp_store=MEMORY");
        q.exec("PRAGMA cache_size=-65536");
        q.exec(create_table_sql);

        QSqlQuery insert_q(dbh);
        insert_q.prepare(insert_sql);

        if (!dbh.transaction())
        {
            return std::unexpected{dbh.lastError().text().toStdString()};
        }

        // a list of columns of bound values
        for (const auto& sample : samples)
        {
            if (!binder(insert_q, sample))
            {
                return std::unexpected{"Failed to bind sample"};
            }

            if (!insert_q.exec())
            {
                dbh.rollback();
                return std::unexpected{insert_q.lastError().text().toStdString()};
            }
        }

        if (!dbh.commit())
        {
            return std::unexpected{dbh.lastError().text().toStdString()};
        }

        // fold the WAL back into the main file so it's self-contained on disk
        {
            QSqlQuery q(dbh);
            q.exec("PRAGMA wal_checkpoint(TRUNCATE)");
        }

        insert_q.finish();
        dbh.close();
    }

    QSqlDatabase::removeDatabase("conn");
    return {};
}

std::expected<void, std::string>
log_solver_run_stats(const std::string& dsn, const std::vector<solver_run_stats>& samples)
{
    static const QString create_table_sql{"CREATE TABLE IF NOT EXISTS solver_run("
                                          " run_id INTEGER NOT NULL,"
                                          " runtime_ms INTEGER NOT NULL,"
                                          " field_config_name VARCHAR,"
                                          " rows INTEGER NOT NULL,"
                                          " columns INTEGER NOT NULL,"
                                          " landmine_fill_rate DOUBLE NOT NULL,"
                                          " seed INTEGER NOT NULL,"
                                          // initial_pois: do not add
                                          " final_uncovered_positions INTEGER NOT NULL,"
                                          " final_landmines_marked INTEGER NOT NULL, "

                                          " PRIMARY KEY(run_id)) WITHOUT ROWID"};

    static const QString insert_row_sql{"INSERT INTO solver_run VALUES (?,?,?,?,?,?,?,?,?)"};

    return log_samples<solver_run_stats>(
      dsn,
      samples,
      create_table_sql,
      insert_row_sql,
      [](QSqlQuery& q, const solver_run_stats& s) -> bool
      {
          q.addBindValue(qlonglong(s.run_id));
          q.addBindValue(qlonglong(s.runtime_ms.count()));
          q.addBindValue(QString::fromUtf8(s.field_config_.name));
          q.addBindValue(qlonglong(s.field_config_.rows));
          q.addBindValue(qlonglong(s.field_config_.columns));
          q.addBindValue(s.field_config_.landmine_fill_rate);
          q.addBindValue(qlonglong(s.field_config_.seed));
          q.addBindValue(qlonglong(s.field_config_.final_uncovered_positions));
          q.addBindValue(qlonglong(s.field_config_.final_landmines_marked));

          return true;
      });
}

std::expected<void, std::string>
log_solver_step_stats(const std::string& dsn, const std::vector<solver_step_stats>& samples)
{
    static const QString create_table_sql{"CREATE TABLE IF NOT EXISTS solver_step("
                                          " run_id INTEGER NOT NULL,"
                                          " at INTEGER NOT NULL,"
                                          " frontier_size INTEGER NOT NULL,"
                                          " uncovered_count INTEGER NOT NULL,"
                                          " marked_count INTEGER NOT NULL,"

                                          " PRIMARY KEY(run_id, at)) WITHOUT ROWID"};

    static const QString insert_row_sql{"INSERT INTO solver_step VALUES (?,?,?,?,?)"};

    return log_samples<solver_step_stats>(
      dsn,
      samples,
      create_table_sql,
      insert_row_sql,
      [](QSqlQuery& q, const solver_step_stats& s) -> bool
      {
          q.addBindValue(qlonglong(s.run_id));
          q.addBindValue(qlonglong(s.at.count()));
          q.addBindValue(qlonglong(s.frontier_size));
          q.addBindValue(qlonglong(s.uncovered_count));
          q.addBindValue(qlonglong(s.marked_count));

          return true;
      });
}

struct name_value
{
    std::string name;
    std::string value;
};

std::expected<void, std::string> log_build_info(const std::string& dsn)
{
    static const QString create_table_sql{"CREATE TABLE IF NOT EXISTS build_info("
                                          " name VARCHAR,"
                                          " value VARCHAR,"
                                          " PRIMARY KEY(name)) WITHOUT ROWID"};

    static const QString insert_row_sql{"INSERT INTO build_info VALUES (?,?)"};

    std::vector<name_value> rows = std::initializer_list<name_value>{
      {"git_sha", build_info::git_sha},
      {"git_describe", build_info::git_describe},
      {"git_dirty", i::to_string(build_info::git_dirty)},
      {"compiler_id", build_info::compiler_id},
      {"compiler_ver", build_info::compiler_ver},
      {"build_type", build_info::build_type},
      {"cxx_flags", build_info::cxx_flags},
      {"cxx_standard", build_info::cxx_standard},
      {"system_name", build_info::system_name},
      {"system_proc", build_info::system_proc},
    };

    return log_samples<name_value>(
      dsn,
      rows,
      create_table_sql,
      insert_row_sql,
      [](QSqlQuery& q, const name_value& s) -> bool
      {
          q.addBindValue(QString::fromUtf8(s.name));
          q.addBindValue(QString::fromUtf8(s.value));

          return true;
      });
}
} // namespace b
