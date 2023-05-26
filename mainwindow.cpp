#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent),
    ui(new Ui::MainWindow),
    benchController(new BenchController(ui, this))
{
}

MainWindow::~MainWindow()
{
    delete ui;
}