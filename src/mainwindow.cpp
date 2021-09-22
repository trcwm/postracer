#include <QScreen>
#include <QApplication>
#include <QMessageBox>
#include <QMenuBar>
#include <iostream>

#include "mainwindow.h"
#include "serialportdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_threadRunning.store(false);

    createActions();
    createMenus();

    m_graph = new Graph(this);
    setCentralWidget(m_graph);

#if 0
    m_serial.reset(SerialCtrl::open("/dev/ttyUSB0", this));

    if (m_serial != nullptr)
    {
        m_serial->setCollectorVoltage(0, true);
        m_serial->setBaseCurrent(500, true);
    }
    else
    {
        std::cout << "ugh!\n";
    }


    if (m_serial)
    {
        m_thread = std::thread([this]()
        {
#if 0            
            for(uint32_t bb = 100; bb < 800; bb += 50)
            {
                m_serial->setBaseCurrent(bb);
                m_serial->sweepCollector(0,1023, 10);
            }
#else
            m_serial->setBaseCurrent(300);
            m_serial->sweepCollector(0,1023, 10);
#endif
        });
    }
#endif


}

MainWindow::~MainWindow()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void MainWindow::createActions()
{
    m_quitAction = new QAction("&Quit");
    connect(m_quitAction, &QAction::triggered, this, &MainWindow::onQuit);

    m_saveAction = new QAction("&Save As...");
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSave);

    m_connectAction = new QAction("Connect");
    connect(m_connectAction, &QAction::triggered, this, &MainWindow::onConnect);

    m_disconnectAction = new QAction("Disconnect");
    connect(m_disconnectAction, &QAction::triggered, this, &MainWindow::onDisconnect);

    m_sweepDiodeAction = new QAction("Diode");
    connect(m_sweepDiodeAction, &QAction::triggered, this, &MainWindow::onSweepDiode);

    m_sweepTransistorAction = new QAction("Ttransistor");
    connect(m_sweepTransistorAction, &QAction::triggered, this, &MainWindow::onSweepTransistor);
    
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(m_saveAction);
    fileMenu->addAction(m_quitAction);

    QMenu *serialMenu = menuBar()->addMenu(tr("Connect"));
    serialMenu->addAction(m_connectAction);
    serialMenu->addAction(m_disconnectAction);

    QMenu *sweepMenu = menuBar()->addMenu(tr("Sweep"));
    sweepMenu->addAction(m_sweepDiodeAction);
    sweepMenu->addAction(m_sweepTransistorAction);
}

bool MainWindow::event(QEvent *event)
{
    if(event->type() == QEvent::User)
    {
        auto evt = static_cast<DataEvent*>(event);
        switch(evt->dataType())
        {
        case DataEvent::DataType::Base:
            handleBaseData(evt->data());
            break;
        case DataEvent::DataType::Collector:
            handleCollectorData(evt->data());
            break;
        case DataEvent::DataType::Diode:
            handleDiodeData(evt->data());
            break;            
        case DataEvent::DataType::EndSweep:
            // add label to the curve
            m_graph->addLabel(QString::asprintf("%.2f uA", m_baseCurrent*1.0e6f),
                m_lastCurvePoint);

            m_graph->newCurve();  
            break;
        default:
            return false;
        }

        return true;        
    }
    
    return QMainWindow::event(event);
}


void MainWindow::handleBaseData(const std::string &data)
{
    int32_t v1,v2;
    sscanf(data.c_str(), "%d\t%d", &v1, &v2);

    m_baseCurrent = (v1-v2) / 3300.0f / 256.0f / 1024.0f * 5.0f;
    std::cout << "Base: " << v1 << " " << v2 << " -> " << m_baseCurrent*1.0e6 << "uA \n";
    std::cout << std::flush;
}

void MainWindow::handleDiodeData(const std::string &data)
{
    int32_t v1,v2;
    sscanf(data.c_str(), "%d\t%d", &v1, &v2);    

    float collectorCurrent = (v1-v2) / 1000.0f / 256.0f / 1024.0f * 5.0f;
    float collectorVoltage = v2 / 256.0f / 1024.0f * 5.0f;    

    m_lastCurvePoint = QPointF(collectorVoltage, collectorCurrent);
    m_graph->addDataPoint(m_lastCurvePoint);    
}

void MainWindow::handleCollectorData(const std::string &data)
{
    int32_t v1,v2;
    sscanf(data.c_str(), "%d\t%d", &v1, &v2);

    float collectorCurrent = (v1-v2) / 1000.0f / 256.0f / 1024.0f * 5.0f;
    float collectorVoltage = v2 / 256.0f / 1024.0f * 5.0f;
    std::cout << "Collector: " << v1 << " " << v2 << " -> " << collectorCurrent*1.0e6 << " uA   voltage: " << collectorVoltage << " V\n";
    std::cout << std::flush;

    m_lastCurvePoint = QPointF(collectorVoltage, collectorCurrent);
    m_graph->addDataPoint(m_lastCurvePoint);
}

void MainWindow::onSave()
{

}

void MainWindow::onQuit()
{
    QApplication::quit();
}

void MainWindow::onConnect()
{
    SerialPortDialog dialog;
    auto status = dialog.exec();
    if (status == QDialog::Accepted)
    {
        auto serialPortLocation = dialog.getSerialPortLocation().toStdString();
        if (!m_serial)        
        {
            auto ctrl = SerialCtrl::open(serialPortLocation, this);
            m_serial.reset(ctrl);

            if (m_serial)
            {
                std::cout << "Serial port " << serialPortLocation << " open ok\n";
            }
            else
            {
                std::cout << "Serial port " << serialPortLocation << " open failure\n";
            }
        }
    }
}

void MainWindow::onDisconnect()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }

    if (m_serial)
    {
        m_serial.reset(nullptr);
        std::cout << "Serial port disconnected\n";
    }
    else
    {
        std::cout << "Serial port already disconnected\n";
    }
}

void MainWindow::onSweepDiode()
{
    if (m_threadRunning.load())
    {
        QMessageBox::critical(this, "Error", "Previous sweep is still running!");
        return;
    }

    m_graph->clearData();

    if (m_serial)
    {   
        m_thread = std::thread([this]()
            {   
                m_threadRunning.store(true);
                m_serial->setBaseCurrent(0, false);
                m_serial->setBaseCurrent(0);
                m_serial->sweepDiode(0,1023, 10);
                m_threadRunning.store(false);
            }
        );
        m_thread.detach();
    }
}

void MainWindow::onSweepTransistor()
{
    if (m_threadRunning.load())
    {
        QMessageBox::critical(this, "Error", "Previous sweep is still running!");
        return;
    }

    m_graph->clearData();

    if (m_serial)
    {   
        m_thread = std::thread([this]()
            {
#if 0            
                for(uint32_t bb = 100; bb < 800; bb += 50)
                {
                    m_serial->setBaseCurrent(bb);
                    m_serial->sweepCollector(0,1023, 10);
                }
#else
                m_threadRunning.store(true);
                m_serial->setBaseCurrent(300, false);
                m_serial->setBaseCurrent(300);
                m_serial->sweepCollector(0,1023, 10);
                m_threadRunning.store(false);
#endif
            }
        );
        m_thread.detach();
    }
}