#include "board.hpp"
#include "game_board_widget.hpp"
#include "main_window.hpp"
#include "glpk_solver.hpp"

#include "ui_main_window.h"
#include "ui_configure_field_dialog.h"

#include <QMessageBox>
#include <QtGui/QAction>

namespace miner
{

MainWindow::~MainWindow() {}

MainWindow::MainWindow() : ui_{new Ui::MainWindow}
{
    ui_->setupUi(this);
    
    ui_->scrollArea->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    game_board_widget_ = new GameBoardWidget;
    ui_->scrollArea->setWidget(game_board_widget_);
    game_board_widget_->show();
    connect(
      game_board_widget_, &GameBoardWidget::cell_changed,
      this, &MainWindow::cell_changed);
    connect(
      game_board_widget_, &GameBoardWidget::game_lost,
      this, &MainWindow::game_lost);
    
    connect(
      ui_->action_About, &QAction::triggered,
      this, &MainWindow::action_about);
    
    //QIcon(":/images/xxx.png")
    auto* a = new QAction("&New", this);
    a->setShortcuts({Qt::CTRL | Qt::Key_N, Qt::Key_F2});
    a->setStatusTip("New field");
    connect(a, &QAction::triggered, this, &MainWindow::gen_new);
    ui_->toolBar->addAction(a);
    
    a = new QAction("&Configure", this);
    a->setShortcuts({Qt::CTRL | Qt::Key_C, Qt::Key_F3});
    a->setStatusTip("Configure field");
    connect(a, &QAction::triggered, this, &MainWindow::configure_field);
    ui_->toolBar->addAction(a);
    
    show_mines_action_ = a = new QAction("Show &mines", this);
    a->setStatusTip("Show mines");
    a->setCheckable(true);
    connect(a, &QAction::toggled, this, &MainWindow::show_mines_toggled);
    ui_->toolBar->addAction(a);
    
    run_solver_action_ = a = new QAction("&Solve", this);
    a->setShortcuts({Qt::Key_Space});
    a->setStatusTip("Solve");
    a->setCheckable(true);
    connect(a, &QAction::toggled, this, &MainWindow::run_solver);
    ui_->toolBar->addAction(a);
    
    a = new QAction("-", this);
    a->setStatusTip("Zoom out");
    a->setShortcuts({Qt::CTRL | Qt::Key_Minus});
    connect(a, &QAction::triggered, game_board_widget_, &GameBoardWidget::zoom_out);
    ui_->toolBar->addAction(a);
    
    a = new QAction("+", this);
    a->setStatusTip("Zoom in");
    a->setShortcuts({Qt::CTRL | Qt::Key_Plus, Qt::CTRL | Qt::Key_Equal});
    connect(a, &QAction::triggered, game_board_widget_, &GameBoardWidget::zoom_in);
    ui_->toolBar->addAction(a);
    
    a = new QAction("Point mode", this);
    a->setStatusTip("Set point mode");
    a->setShortcuts({Qt::CTRL | Qt::Key_0, Qt::Key_P});
    connect(
      a, &QAction::triggered,
      game_board_widget_, &GameBoardWidget::switch_point_mode);
    a->setCheckable(true);
    ui_->toolBar->addAction(a);
    
    mines_info_label_ = new QLabel();
    statusBar()->addPermanentWidget(mines_info_label_);
    
    gen_new();
}

void MainWindow::setup_solver()
{
    auto board = game_board_widget_->board();
    
    solver_.reset(new GlpkSolver{board});
    
    solver_->setResultHandler(
      [this](auto ft, miner::Location l, size_t range)
      {
          // QThread::usleep(0); // slow down a bit for nice animation effect
          QMetaObject::invokeMethod(
            this, [this, ft, l, range] {
                solver_result_slot(ft, l, range);
            }, Qt::QueuedConnection);
      });
    solver_->startAsync();
}

void MainWindow::gen_new()
{
    show_mines_action_->setChecked(false);
    
    auto field = std::make_shared<Field>();
    field->gen_random(new_rows_, new_cols_, new_mines_);
    
    auto board = std::make_shared<GameBoard>();
    board->set_field(field);
    game_board_widget_->set_board(board);
    setup_solver();
    update_cell_info();
    game_board_widget_->set_rw(true);
}

void MainWindow::configure_field()
{
    QDialog d;
    Ui::ConfigureFieldDialog ui;
    ui.setupUi(&d);
    ui.rows_sb->setValue(new_rows_);
    ui.cols_sb->setValue(new_cols_);
    ui.mines_sb->setValue(new_mines_);
    if (!d.exec())
    {
	return;
    }
    
    new_rows_ = ui.rows_sb->value();
    new_cols_ = ui.cols_sb->value();
    new_mines_ = ui.mines_sb->value();
    gen_new();
}

void MainWindow::show_mines_toggled(bool v) {
    game_board_widget_->set_show_mines(v);
}

void MainWindow::run_solver(bool v) {
    if (game_board_widget_->board()->game_lost())
    {
	return;
    }
    
    if (v)
    {
	game_board_widget_->set_rw(false);
	solver_->resume();
    }
    else
    {
	solver_->suspend();
    }
}

void MainWindow::action_about() {
    QMessageBox::about(
      this, "Miner",
      "Miner: a simple mines game with solver.\n"
      "Copyright (C) 2015-2018 Igor Shevchenko <igor.shevchenko@gmail.com>\n"
      "This program comes with ABSOLUTELY NO WARRANTY.\n"
      "This is free software, and you are welcome to redistribute it\n"
      "under certain conditions. Look here for GPL3 license: http://www.gnu.org/licenses/");
}

void MainWindow::cell_changed(miner::Location l)
{
    solver_->addPoi(l);
    update_cell_info();
}

void MainWindow::update_cell_info()
{
    mines_info_label_->setText(
      QString("Mines: %1 / %2 Uncovered: %3 Left: %4")
      .arg(game_board_widget_->board()->mines_marked())
      .arg(game_board_widget_->board()->field()->mines_nr())
      .arg(game_board_widget_->board()->uncovered_nr())
      .arg(game_board_widget_->board()->left_nr()));
}

void MainWindow::game_lost()
{
    game_board_widget_->board()->set_game_lost();
    show_mines_action_->setChecked(true);
}

void MainWindow::solver_result_slot(
  Solver::FeedbackState feedback_state,
  miner::Location l, size_t range)
{
    switch(feedback_state) {
    case Solver::FeedbackState::kSolved:
	game_board_widget_->update_box(l, range);
	update_cell_info();
	break;
	
    case Solver::FeedbackState::kSuspended:
	game_board_widget_->set_rw(true);
	run_solver_action_->setChecked(false);
	break;
	
    case Solver::FeedbackState::kGameLost:
	run_solver_action_->setChecked(false);
	game_lost();
	break;
    };
}

} // namespace miner
