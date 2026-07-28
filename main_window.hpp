#pragma once

#include "field.hpp"
#include "solver.hpp"

#include <QMainWindow>
#include <QLabel>

namespace Ui { class MainWindow; }

namespace sweeper
{

class GameBoardWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT;
public:
    MainWindow();
    ~MainWindow();
                 
private slots:
    void action_about();
    void generate_new();
    void configure_field();
    void show_landmines_toggled(bool);
    void run_solver(bool);
    void cell_changed(qed::FieldPosition);
    void game_lost();
    void
    handle_solver_result(qed::Solver::SolverState, qed::FieldPosition, const std::string& errmsg);

private:
    void update_cell_info();
    void setup_solver();
    
    std::unique_ptr<Ui::MainWindow> ui_;
    GameBoardWidget* game_board_widget_{};
    std::unique_ptr<qed::Solver> solver_{};

    qed::index_type new_rows_{3};
    qed::index_type new_columns_{3};
    qed::index_type new_landmines_{2};

    QAction* run_solver_action_{};
    QAction* show_landmines_action_{};
    QLabel* landmines_info_label_{};
};

} // namespace sweeper
