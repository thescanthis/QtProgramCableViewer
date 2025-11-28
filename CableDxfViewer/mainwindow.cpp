#include "pch.h"
#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_view = new DxfView(this);
    setCentralWidget(m_view);

    resize(1024,768);

    m_view->loadDxfFile("C:/AIND_Library/Git_PredicTest/x64_Cami_Lib/QtProgramCableViewer/CableDxfViewer/Testdxl/A20016147.dxf");
}

MainWindow::~MainWindow()
{
    delete ui;
}
