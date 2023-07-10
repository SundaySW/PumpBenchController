#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      benchController_(new BenchController(ui, this))
{
}

MainWindow::~MainWindow()
{
    delete ui;
}