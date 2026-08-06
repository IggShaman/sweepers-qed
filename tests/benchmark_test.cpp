#include "benchmark_stats.hpp"
#include "logger.hpp"

#include <catch2/catch_test_macros.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QTemporaryDir>

struct QtCoreApp
{
    int argc = 1;
    char arg0[6] = "test";
    char* argv[2] = {arg0, nullptr};
    QCoreApplication app{argc, argv};
};

TEST_CASE("log_solver_run_stats")
{
    QtCoreApp qapp;

    QTemporaryDir temp_dir(QDir::tempPath() + "/bench-XXXXXX");
    REQUIRE(temp_dir.isValid());
    const QString dsn = temp_dir.filePath("bench.sqlite");

    // temp_dir.setAutoRemove(false);
    // system(I_TO_STRING("ls -al " << dsn.toStdString()).data());

    std::vector<qed::solver_run_stats> samples;
    samples.push_back(
      {.run_id = 10,
       .runtime_ms = qed::ms_duration_type{500},
       .field_config_ = {
         .name = "test name",
         .rows = 1,
         .columns = 2,
         .landmine_fill_rate = 0.1,
         .seed = 1ull << 63,
         .final_uncovered_positions = 10,
         .final_landmines_marked = 20}});
    samples.push_back({
      .run_id = 20,
      .runtime_ms = qed::ms_duration_type{1500},
      .field_config_ =
        {.name = "test name 2",
         .rows = 11,
         .columns = 21,
         .landmine_fill_rate = 0.11,
         .seed = (1ull << 63) + 1,
         .final_uncovered_positions = (1ull << 63) + 2,
         .final_landmines_marked = (1ull << 63) + 3},
    });

    auto ok = qed::log_solver_run_stats(dsn.toStdString(), samples);
    if (!ok)
    {
        errlog << ok.error() << "\n";
        REQUIRE(ok);
    }

    {
        auto dbh = QSqlDatabase::addDatabase("QSQLITE", "conn");
        dbh.setDatabaseName(dsn);
        REQUIRE(dbh.open());

        QSqlQuery q(dbh);
        REQUIRE(q.prepare("select * from solver_run order by run_id"));
        REQUIRE(q.exec());

        REQUIRE(q.next());
        REQUIRE(q.value(0) == 10);
        REQUIRE(q.value(1).toULongLong() == 500);
        REQUIRE(q.value(2).toString() == "test name");
        REQUIRE(q.value(3).toInt() == 1);
        REQUIRE(q.value(4).toInt() == 2);
        REQUIRE(q.value(5).toDouble() == 0.1);
        REQUIRE(q.value(6).toULongLong() == 1ull << 63);
        REQUIRE(q.value(7).toULongLong() == 10);
        REQUIRE(q.value(8).toULongLong() == 20);

        REQUIRE(q.next());
        REQUIRE(q.value(0) == 20);
        REQUIRE(q.value(1).toULongLong() == 1500);
        REQUIRE(q.value(2).toString() == "test name 2");
        REQUIRE(q.value(3).toInt() == 11);
        REQUIRE(q.value(4).toInt() == 21);
        REQUIRE(q.value(5).toDouble() == 0.11);
        REQUIRE(q.value(6).toULongLong() == (1ull << 63) + 1);
        REQUIRE(q.value(7).toULongLong() == (1ull << 63) + 2);
        REQUIRE(q.value(8).toULongLong() == (1ull << 63) + 3);

        REQUIRE(!q.next());
    }

    QSqlDatabase::removeDatabase("conn");
}

TEST_CASE("log_solver_step_stats")
{
    QtCoreApp qapp;

    QTemporaryDir temp_dir(QDir::tempPath() + "/bench-XXXXXX");
    REQUIRE(temp_dir.isValid());
    const QString dsn = temp_dir.filePath("bench.sqlite");

    // temp_dir.setAutoRemove(false);
    // system(I_TO_STRING("ls -al " << dsn.toStdString()).data());

    std::vector<qed::solver_step_stats> samples;
    samples.push_back({
      .run_id = 1,
      .at = qed::ms_duration_type{100},
      .frontier_size = 1,
      .uncovered_count = 0,
      .marked_count = 2,
    });
    samples.push_back({
      .run_id = 1,
      .at = qed::ms_duration_type{200},
      .frontier_size = 2,
      .uncovered_count = 3,
      .marked_count = 4,
    });
    samples.push_back({
      .run_id = 2,
      .at = qed::ms_duration_type{1},
      .frontier_size = 100,
      .uncovered_count = 200,
      .marked_count = 300,
    });

    auto ok = qed::log_solver_step_stats(dsn.toStdString(), samples);
    if (!ok)
    {
        errlog << ok.error() << "\n";
        REQUIRE(ok);
    }

    {
        auto dbh = QSqlDatabase::addDatabase("QSQLITE", "conn");
        dbh.setDatabaseName(dsn);
        REQUIRE(dbh.open());

        QSqlQuery q(dbh);
        REQUIRE(q.prepare("select * from solver_step order by run_id,at"));
        REQUIRE(q.exec());

        REQUIRE(q.next());
        REQUIRE(q.value(0) == 1);
        REQUIRE(q.value(1).toULongLong() == 100);
        REQUIRE(q.value(2).toULongLong() == 1);
        REQUIRE(q.value(3).toULongLong() == 0);
        REQUIRE(q.value(4).toULongLong() == 2);

        REQUIRE(q.next());
        REQUIRE(q.value(0) == 1);
        REQUIRE(q.value(1).toULongLong() == 200);
        REQUIRE(q.value(2).toULongLong() == 2);
        REQUIRE(q.value(3).toULongLong() == 3);
        REQUIRE(q.value(4).toULongLong() == 4);

        REQUIRE(q.next());
        REQUIRE(q.value(0) == 2);
        REQUIRE(q.value(1).toULongLong() == 1);
        REQUIRE(q.value(2).toULongLong() == 100);
        REQUIRE(q.value(3).toULongLong() == 200);
        REQUIRE(q.value(4).toULongLong() == 300);

        REQUIRE(!q.next());
    }

    QSqlDatabase::removeDatabase("conn");
}
