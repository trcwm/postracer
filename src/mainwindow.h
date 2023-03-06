#pragma one

#include <thread>
#include <QMainWindow>
#include <QListWidget>
#include <QAction>
#include <QTimer>

#include "messagetypes.h"
#include "messagequeue.h"
#include "serialctrl.h"
#include "graph.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

signals:

public slots:
    void onSweepSetup();
    void onSweepDiode();
    void onSweepTransistor();
    void onConnect();
    void onDisconnect();
    void onQuit();
    void onSave();
    void onPersistanceChanged();
    void onSelectedTraceChanged();
    void onClearTraces();
    void onAbout();
    void onMeasurementTimer();

protected:
    void createNewTrace();
    void createMenus();
    void createActions();
    
    QAction *m_quitAction;
    QAction *m_saveAction;
    QAction *m_connectAction;
    QAction *m_disconnectAction;
    QAction *m_sweepTransistorAction;
    QAction *m_sweepDiodeAction;
    QAction *m_persistanceAction;
    QAction *m_sweepSetupAction;
    QAction *m_clearTracesAction;
    QAction *m_aboutAction;

    QPointF m_lastCurvePoint;
    
    Messages::SweepSetup m_sweepSetup;
    bool    m_persistance;

    Graph *m_graph;
    QListWidget *m_traceList;

    QTimer *m_timer = nullptr;

    MessageQueue<Messages::DataPoint> m_traceResults;

    std::unique_ptr<SerialCtrl> m_serial;
};

