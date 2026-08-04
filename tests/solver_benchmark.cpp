#include "logger.hpp"
#include "tests/benchmark_options.hpp"
#include "tests/benchmark_runner.hpp"

int main(int argc, char** argv)
{
    const auto cli_opts = b::get_cli_options(argc, argv);
    if (!cli_opts)
    {
        exit(-1);
    }

    b::benchmark_runner runner;
    auto ok = runner.run();
    if (!ok)
    {
        errlog << ok.error() << "\n";
        return -1;
    }

    return 0;
}
