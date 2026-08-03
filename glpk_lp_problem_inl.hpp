namespace lp {

inline void matrix::add(int row, int col, double value)
{
    rows_.push_back(row);
    columns_.push_back(col);
    values_.push_back(value);
}

inline std::expected<void, int> problem::solve()
{
    auto error_code = glp_simplex(glp_, &glp_opt_);
    if (get_status() == status::kOPT)
    {
        return {};
    }
    return std::unexpected{error_code};
}

} // namespace lp
