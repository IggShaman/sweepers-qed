#include "benchmark_options.hpp"
#include "benchmark_runner.hpp"
#include "logger.hpp"

#include <QCoreApplication>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    const auto cli_opts = b::get_cli_options(argc, argv);
    if (!cli_opts)
    {
        exit(-1);
    }

    tlog << "experiment_path: " << cli_opts->experiment_path << "\n";
    tlog << "experiment_name: " << cli_opts->experiment_name << "\n";

    b::benchmark_runner runner;
    if (auto ok = runner.init(*cli_opts); !ok)
    {
        errlog << ok.error() << "\n";
        return -1;
    }
    auto ok = runner.run();
    if (!ok)
    {
        errlog << ok.error() << "\n";
        return -1;
    }

    return 0;
}
