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

    void on_maxLevel_valueChanged(double arg1);

    void on_minLevel_valueChanged(double arg1);

    void on_startFreqComboBox_currentIndexChanged(int index);

    void on_centerFreq_editingFinished();

    void on_startFreq_editingFinished();

    void on_spanFreq_editingFinished();

    void on_endFreq_editingFinished();

private:

    bool _init;

    Ui::MainWindow *_ui;
    D6 _d6;

    QChart *_chart;

    bool _continuousScanning;
    void doScan();

    void setupInitialRangeValues();
    void updateHorizontalRange();
    void updateVerticalRange();
    void setStepSize(double step_size);
    void recomputeStepSize();
};
#endif // MAINWINDOW_H
