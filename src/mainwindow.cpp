#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <QScreen>
#include <QApplication>
#include <QMessageBox>
#include <QMenuBar>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QVariant>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QDebug>

#include "mainwindow.h"
#include "serialportdialog.h"
#include "sweepdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    createActions();
    createMenus();

    auto mainWidget = new QWidget();
    setCentralWidget(mainWidget);

    auto hLayout = new QHBoxLayout();
    mainWidget->setLayout(hLayout);

    m_traceList = new QListWidget();
    connect(m_traceList, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectedTraceChanged()) );
    hLayout->addWidget(m_traceList, 1);

    m_graph = new Graph(this);
    m_graph->selectTrace(0);
    hLayout->addWidget(m_graph, 5);  

    memset(&m_sweepSetup, 0, sizeof(m_sweepSetup));
    m_sweepSetup.m_diode.m_startCurrent = 0;
    m_sweepSetup.m_diode.m_stopCurrent = 5e-3;
    m_sweepSetup.m_points = 50;

    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onMeasurementTimer);
    m_timer->start(250);
}

MainWindow::~MainWindow()
{
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

    m_sweepTransistorAction = new QAction("Transistor");
    connect(m_sweepTransistorAction, &QAction::triggered, this, &MainWindow::onSweepTransistor);
    
    m_persistanceAction = new QAction("Trace persistance");
    m_persistanceAction->setCheckable(true);
    m_persistanceAction->setChecked(false);
    connect(m_persistanceAction, &QAction::triggered, this, &MainWindow::onPersistanceChanged);

    m_sweepSetupAction = new QAction("Setup");
    connect(m_sweepSetupAction, &QAction::triggered, this, &MainWindow::onSweepSetup);

    m_clearTracesAction = new QAction("Clear traces");
    connect(m_clearTracesAction, &QAction::triggered, this, &MainWindow::onClearTraces);

    m_aboutAction = new QAction("About");
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
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
    sweepMenu->addAction(m_sweepSetupAction);
    sweepMenu->addAction(m_sweepDiodeAction);
    sweepMenu->addAction(m_sweepTransistorAction);
    sweepMenu->addSeparator();
    sweepMenu->addAction(m_clearTracesAction);
    sweepMenu->addAction(m_persistanceAction);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::onSave()
{
    // save the traces as JSON file
    auto filename = QFileDialog::getSaveFileName(this, tr("Save traces as JSON"), "", tr("JSON files (*.json)"));

    if (!filename.isEmpty())
    {
        std::ofstream json(filename.toStdString());

        if (!json.is_open())
        {
            return;
        }

        json << "{\n";

        size_t index = 1;
        for(auto & trace : m_graph->traces())
        {
            std::stringstream ss;
            ss << "    \"trace" << index << "\": [";
            json << ss.str();

            bool first = true;
            for(auto &pt : trace.m_data)
            {
                if (!first) 
                {
                    json << ",";
                }

                json << "[" << pt.x() << " ," << pt.y() << "]";
                first = false;
            }

            if (index != m_graph->traces().size())
            {
                json << "],\n";
            }    
            else
            {
                json << "]\n";
            }
            
            index++;
        }

        json << "}\n";
    }
}

void MainWindow::onQuit()
{
    QApplication::quit();
}

void MainWindow::onPersistanceChanged()
{
    m_persistance = m_persistanceAction->isChecked();
    std::cout << "Persistance = " << m_persistance << "\n";
}

void MainWindow::onSweepSetup()
{
    SweepDialog dialog(m_sweepSetup);
    auto status = dialog.exec();
    if (status == QDialog::Accepted)
    {
        m_sweepSetup = dialog.getSetup();
    }
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
            auto ctrl = SerialCtrl::open(serialPortLocation, m_traceResults);
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

void MainWindow::onMeasurementTimer()
{
    while (m_traceResults.hasMessages())
    {
        auto msg = m_traceResults.pop();

        // diode only
        // base current, versus base voltage
        QPointF p(msg.m_baseCurrent, msg.m_baseVoltage);
        m_graph->addDataPoint(p);
    }
}

void MainWindow::onSweepTransistor()
{
    if (!m_persistance)
    {
        m_graph->clearData();
        m_traceList->clear();
    }

    m_sweepSetup.m_sweepType = Messages::SweepType::BJT_Collector;
    m_graph->setUnitStrings("V", "V");
    createNewTrace();
    m_serial->doSweep(m_sweepSetup);  
}

void MainWindow::onSweepDiode()
{
    if (!m_persistance)
    {
        m_graph->clearData();
        m_traceList->clear();
    }

    m_sweepSetup.m_sweepType = Messages::SweepType::Diode;
    m_graph->setUnitStrings("A", "V");
    createNewTrace();
    m_serial->doSweep(m_sweepSetup);
}

void MainWindow::onSelectedTraceChanged()
{
    auto item = m_traceList->currentItem();
    if (item != nullptr)
    {
        QVariant userData = item->data(Qt::UserRole);
        int32_t traceIndex = userData.toInt() - 1;
        m_graph->selectTrace(traceIndex);
    }
}

void MainWindow::onClearTraces()
{
    m_graph->clearData();
    m_traceList->clear();
}

void MainWindow::onAbout()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::createNewTrace()
{
    auto numberOfTraces = m_graph->newTrace();
    auto item = new QListWidgetItem();
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setData(Qt::DisplayRole, QString::asprintf("Trace %ld", numberOfTraces));
    item->setData(Qt::UserRole, static_cast<int>(numberOfTraces));

    QImage image(12,12, QImage::Format::Format_RGB888);
    QPainter painter(&image);
    painter.setPen(Qt::black);
    painter.setBrush(QColor(m_graph->traces().back().m_color));
    auto r = image.rect();
    r.adjust(0,0,-1,-1);
    painter.drawRect(r);
    auto pixmap = QPixmap::fromImage(image);
    item->setIcon(QIcon(pixmap));

    m_traceList->addItem(item);

    if (numberOfTraces == 1)
    {
        m_graph->selectTrace(0);
    }
}
