#pragma once

#include "field.hpp"
#include "solver.hpp"

#include <QMainWindow>
#include <QLabel>

namespace Ui { class MainWindow; }

namespace landmine {

class GameBoardWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT;
public:
    MainWindow();
    ~MainWindow();
                 
private slots:
    void action_about();
    void gen_new();
    void configure_field();
    void show_landmines_toggled(bool);
    void run_solver(bool);
    void cell_changed(landmine::FieldPosition);
    void game_lost();
    void solver_result_slot(landmine::Solver::FeedbackState, landmine::FieldPosition center,
                            size_t range);

private:
    void update_cell_info();
    void setup_solver();
    
    std::unique_ptr<Ui::MainWindow> ui_;
    GameBoardWidget* game_board_widget_{};
    std::unique_ptr<Solver> solver_{};
    
    size_t new_rows_{3};
    size_t new_cols_{3};
    size_t new_landmines_{2};

    QAction* run_solver_action_{};
    QAction* show_landmines_action_{};
    QLabel* landmines_info_label_{};
};

} // namespace landmine
