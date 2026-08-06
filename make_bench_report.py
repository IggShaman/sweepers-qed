#!/usr/bin/env -S uv run --quiet --with duckdb --with plotly --with pandas --script
import sys
import duckdb
import plotly.graph_objects as go
from plotly.subplots import make_subplots


def main(dsn, output_file):
    METRICS = [("frontier", "frontier size"),
               ("uncovered", "uncovered"),
               ("marked", "marked")]

    con = duckdb.connect()
    con.execute(f"ATTACH '{dsn}' AS b (TYPE sqlite)")

    raw = con.execute("""
    SELECT step.run_id,
      run.solver_name,
      run.layout_name,
      step.at/1e3 AS at_s,
      step.frontier_size as frontier,
      step.uncovered_count as uncovered,
      step.marked_count as marked
    FROM b.solver_step step JOIN b.solver_run run USING (run_id)
    ORDER BY step.run_id, step.at
    """).df()

    agg = con.execute("""
        SELECT run.solver_name,
               floor(step.at / 50) * 50 / 1e3 AS at_s, -- 50 ms buckets
               count(*) AS n,
               quantile_cont(step.frontier_size, 0.5)  AS frontier_p50,
               quantile_cont(step.frontier_size, 0.10) AS frontier_lo,
               quantile_cont(step.frontier_size, 0.90) AS frontier_hi,
               quantile_cont(step.uncovered_count, 0.5)  AS uncovered_p50,
               quantile_cont(step.uncovered_count, 0.10) AS uncovered_lo,
               quantile_cont(step.uncovered_count, 0.90) AS uncovered_hi,
               quantile_cont(step.marked_count,    0.5)  AS marked_p50,
               quantile_cont(step.marked_count,    0.10) AS marked_lo,
               quantile_cont(step.marked_count,    0.90) AS marked_hi
        FROM b.solver_step step JOIN b.solver_run run USING (run_id)
        GROUP BY 1, 2 HAVING count(*) >= 5 ORDER BY 1, 2
    """).df()

    COLOR = {"glpk": "#4C78A8", "highs": "#F58518"}

    fig = make_subplots(rows=len(METRICS), cols=1, shared_xaxes=True,
                        subplot_titles=[t for _, t in METRICS], vertical_spacing=0.06)

    for row, (col, _) in enumerate(METRICS, start=1):
        for sid, g in raw.groupby("run_id", sort=False):          # spaghetti
            fig.add_trace(go.Scattergl(
                x=g["at_s"], y=g[col], mode="lines",
                line=dict(width=0.6, color="rgba(130,130,130,0.25)"),
                hoverinfo="skip", showlegend=False), row=row, col=1)

        for solver, g in agg.groupby("solver_name", sort=False):  # band + median
            c = COLOR.get(solver, "#888")
            fig.add_trace(go.Scattergl(
                x=list(g["at_s"]) + list(g["at_s"])[::-1],
                y=list(g[f"{col}_hi"]) + list(g[f"{col}_lo"])[::-1],
                fill="toself", fillcolor=c, opacity=0.18, line=dict(width=0),
                hoverinfo="skip", showlegend=False, legendgroup=solver), row=row, col=1)
            fig.add_trace(go.Scattergl(
                x=g["at_s"], y=g[f"{col}_p50"], mode="lines",
                line=dict(width=2.5, color=c), name=f"{solver} median",
                legendgroup=solver, showlegend=(row == 1)), row=row, col=1)

    fig.update_xaxes(title_text="time since solve start (s)", row=len(METRICS), col=1)
    fig.update_layout(height=320 * len(METRICS), width=1100,
                      hovermode="x unified", template="plotly_white",
                      legend=dict(orientation="h", yanchor="bottom", y=1.02))

    print(f"saving report to {output_file=}")
    fig.write_html(output_file, include_plotlyjs="cdn")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        raise Exception(f"Usage: $0 bench.sqlite output.html")
    _, dsn, output_file = sys.argv;
    main(dsn, output_file)
