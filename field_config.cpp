#include "field_config.hpp"
#include "logger.hpp"

#include <toml++/toml.hpp>

namespace b
{
std::vector<qed::field_position> load_pois(const toml::node_view<const toml::node>& n)
{
    std::vector<qed::field_position> out;
    const auto* array = n.as_array();
    // no key => empty
    if (!array)
    {
        return out;
    }

    for (const auto& node : *array)
    {
        const auto* pair = node.as_array();
        if (pair and pair->size() == 2)
        {
            out.push_back(
              {static_cast<qed::index_type>((*pair)[0].value_or(0)),
               static_cast<qed::index_type>((*pair)[1].value_or(0))});
        }
        else
        {
            errlog << "Malformed toml config at: " << n << "\n";
            abort();
        }
    }
    return out;
}

std::expected<std::vector<field_config>, std::string> load_field_configs(std::string file_name)
{
    toml::table doc;
    try
    {
        doc = toml::parse_file(file_name);
    }
    catch (const toml::parse_error& ex)
    {
        const auto& pos = ex.source().begin;
        return std::unexpected{
          i::to_string(file_name, ":", pos.line, ":", pos.column, ": ", ex.description())};
    }

    std::vector<field_config> out;

    if (const auto* array = doc["config"].as_array())
    {
        for (const auto& node : *array)
        {
            const auto& t = *node.as_table();
            out.push_back({
              .name = std::string{t["name"].value_or("")},
              .rows = static_cast<qed::index_type>(t["rows"].value_or(0)),
              .columns = static_cast<qed::index_type>(t["columns"].value_or(0)),
              .landmine_fill_rate = t["landmine_fill_rate"].value_or(0.0),
              .seed = static_cast<std::uint64_t>(t["seed"].value_or(0)),
              .initial_pois = load_pois(t["initial_pois"]),
              .final_uncovered_positions =
                static_cast<std::size_t>(t["final_uncovered_positions"].value_or(0)),
              .final_landmines_marked =
                static_cast<std::size_t>(t["final_landmines_marked"].value_or(0)),
            });
        }
    }
    return out;
}

toml::array pois_as_pairs(const std::vector<qed::field_position>& pois)
{
    toml::array a;
    for (const auto& p : pois)
    {
        a.push_back(toml::array{p.row, p.column});
    }
    return a;
}

std::expected<void, std::string>
save_field_configs(std::string file_name, const std::vector<field_config>& field_configs)
{
    toml::array configs;
    for (const auto& c : field_configs)
    {
        configs.push_back(
          toml::table{
            {"name", c.name},
            {"rows", c.rows},
            {"columns", c.columns},
            {"landmine_fill_rate", c.landmine_fill_rate},
            {"seed", static_cast<int64_t>(c.seed)},
            {"initial_pois", pois_as_pairs(c.initial_pois)},
            {"final_uncovered_positions", static_cast<int64_t>(c.final_uncovered_positions)},
            {"final_landmines_marked", static_cast<int64_t>(c.final_landmines_marked)}});
    }

    toml::table doc{{"config", std::move(configs)}};
    std::ofstream{file_name} << doc << '\n';

    return {};
}
} // namespace b
