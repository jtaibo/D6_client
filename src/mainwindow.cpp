#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <float.h>

void configureFreqUnitsComboBox(QComboBox *cb)
{
    cb->addItem("Hz", 1);
    cb->addItem("kHz", 1e3);
    cb->addItem("MHz", 1e6);
    cb->addItem("GHz", 1e9);
    cb->setCurrentIndex(2);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ui(new Ui::MainWindow)
    , _continuousScanning(false)
{
    _ui->setupUi(this);

    std::string port_device = Serial::getFirstAvailableDevice();
    std::cout << "port device: " << port_device << std::endl;
    _ui->portDevice->setText( QString::fromStdString( port_device ));
    _ui->baudRate->addItem("9600", B9600);
    _ui->baudRate->addItem("19200", B19200);
    _ui->baudRate->addItem("38400", B38400);
    _ui->baudRate->addItem("57600", B57600);
    _ui->baudRate->addItem("115200", B115200);
    _ui->baudRate->setCurrentIndex(3);

    _ui->startFreq->setValue(2019);
    configureFreqUnitsComboBox(_ui->startFreqComboBox);
    _ui->stepSize->setValue(1);
    configureFreqUnitsComboBox(_ui->stepSizeComboBox);
    _ui->samples->setValue(501);

    _ui->vfoFreq->setValue(2400);
    configureFreqUnitsComboBox(_ui->vfoUnitsComboBox);

    // TO-DO: Move to a chart/graph/plot class
    _chart = new QChart();

    _chart->legend()->hide();
    _chart->createDefaultAxes();
//    _chart->setTitle("D6 client");

    _ui->fftView->setChart(_chart);
    _ui->fftView->setRenderHint(QPainter::Antialiasing);

    /*
    _ui->fftView->xAxis->setLabel("Frequency (Hz)");
    _ui->fftView->yAxis->setLabel("Power (dB)");
    // TO-DO: set power (dB, dBm) scale
//    _ui->fftView->yAxis->setRange(0, USHRT_MAX);
    _ui->fftView->yAxis->setRange(0, 500);
    */
}

MainWindow::~MainWindow()
{
    delete _ui;
}


void MainWindow::on_connectButton_clicked()
{
    bool conn = _d6.connect(_ui->portDevice->text().toStdString(), _ui->baudRate->currentData().toUInt());
    if(conn) {
        _ui->connStatus->setText("Connected");
        {
            // Paint on green status bar to show connection state
            QPalette palette;
            palette.setColor(QPalette::Base, Qt::green);
            palette.setColor(QPalette::Text, Qt::black);
            _ui->connStatus->setPalette(palette);
        }
        std::cout << "Connected!" << std::endl;
        std::string ver = _d6.getVersionString();
        std::cout << "Firmware version: " << ver << std::endl;
        _ui->firmwareVersion->setText(ver.c_str());

        const D6::Status &status = _d6.queryStatus();
        std::cout << "Variant: " << (int)status.variant << std::endl;
        std::cout << "Attenuator: " << (int)status.attenuator << std::endl;
        std::cout << "Converter AD: " << std::hex << (int)status.converterAD << std::dec << std::endl;
    }
    else {
        _ui->connStatus->setText("Connection error");
        {
            // Paint on red status bar to show connection state
            QPalette palette;
            palette.setColor(QPalette::Base, Qt::red);
            palette.setColor(QPalette::Text, Qt::black);
            _ui->connStatus->setPalette(palette);
        }
    }
}


void MainWindow::on_scanButton_clicked()
{
    if(!_continuousScanning)
        doScan();
}

void MainWindow::on_setVFOButton_clicked()
{
    if(_d6.connected()) {
        long freq = _ui->vfoFreq->value() * _ui->vfoUnitsComboBox->currentData().toFloat();
        _d6.setVFOFrequency(freq);
    }
}

void MainWindow::on_continuousScanButton_clicked()
{
    _continuousScanning = true;
    doScan();
}

void MainWindow::doScan()
{
    if(_d6.connected()) {
        // Scan data
        std::cout << "SCANNING..." << std::endl;
        _d6.setInterval(_ui->startFreq->value() * _ui->startFreqComboBox->currentData().toFloat(),
                        _ui->stepSize->value() * _ui->stepSizeComboBox->currentData().toFloat(),
                        _ui->samples->value(),
                        0,  // delay (to-do: add to interface)
                        _ui->logCheckBox->checkState() == Qt::Checked
                        );
        _d6.sweep();
        std::cout << "DONE" << std::endl;

#if 0
        // Show data
        QVector<double> x_data(_d6.getNumberOfSamples());
        QVector<double> y1_data(_d6.getNumberOfSamples());
        QVector<double> y2_data(_d6.getNumberOfSamples());
        double min_y=DBL_MAX;
        double max_y=DBL_MIN;
        for(int i=0 ; i < _d6.getNumberOfSamples() ; i++ ) {
            x_data[i] = _d6.getXData()[i];
            y1_data[i] = _d6.getYData()[i*2];
            y2_data[i] = _d6.getYData()[i*2+1];
            if(y1_data[i] > max_y)
                max_y = y1_data[i];
            if(y1_data[i] < min_y)
                min_y = y1_data[i];
            if(y2_data[i] > max_y)
                max_y = y2_data[i];
            if(y2_data[i] < min_y)
                min_y = y2_data[i];
        }
        std::cout << "MIN: " << x_data[0] << "," << min_y
                  << "MAX: " << x_data[_d6.getNumberOfSamples()-1] << "," << max_y << std::endl;

        // TO-DO: adjust labels to scale (Hz, kHz, MHz, GHz)

        _ui->fftView->xAxis->setRange(x_data[0], x_data[_d6.getNumberOfSamples()-1]);
        _ui->fftView->xAxis->setLabel("Freq. (" + QString::fromStdString(_d6.getXUnits()) + ")");

        _ui->fftView->graph(0)->setData(x_data, y1_data);
//        _ui->fftView->graph(1)->setData(x_data, y2_data);
//        _ui->fftView->graph(0)->tip
        _ui->fftView->replot();
#else
        QLineSeries *series = new QLineSeries();

        double min_y=DBL_MAX;
        double max_y=DBL_MIN;
        for(int i=0 ; i < _d6.getNumberOfSamples() ; i++ ) {

            double x = _d6.getXData()[i];
            double y = _d6.getYData()[i*2];
            series->append(x, y);
            if(y > max_y)
                max_y = y;
            if(y < min_y)
                min_y = y;
        }
        std::cout << "MIN: " << _d6.getXData()[0] << "," << min_y
                  << "MAX: " << _d6.getXData()[_d6.getNumberOfSamples()-1] << "," << max_y << std::endl;

        // TO-DO: adjust labels to scale (Hz, kHz, MHz, GHz)

//        _ui->fftView->xAxis->setRange(x_data[0], x_data[_d6.getNumberOfSamples()-1]);
//        _ui->fftView->xAxis->setLabel("Freq. (" + QString::fromStdString(_d6.getXUnits()) + ")");

        _chart->removeAllSeries();
        _chart->addSeries(series);
        _chart->createDefaultAxes();
#endif
    }
    else {
        _chart->removeAllSeries();
    }
}
