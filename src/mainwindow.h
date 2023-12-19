#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "d6.h"
#include <QtCharts>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();

    void on_scanButton_clicked();

    void on_setVFOButton_clicked();

    void on_continuousScanButton_clicked();

private:
    Ui::MainWindow *_ui;
    D6 _d6;

    QChart *_chart;

    bool _continuousScanning;
    void doScan();
};
#endif // MAINWINDOW_H
