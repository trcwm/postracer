#pragma one

#include <thread>
#include <QMainWindow>
#include <QListWidget>
#include <QAction>

#include "customevent.h"
#include "serialctrl.h"
#include "graph.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

    bool event(QEvent *event) override;

signals:

public slots:
    void onSweepDiode();
    void onSweepTransistor();
    void onConnect();
    void onDisconnect();
    void onQuit();
    void onSave();
    void onPersistanceChanged();
    void onSelectedTraceChanged();

protected:
    void handleBaseData(const std::string &data);
    void handleCollectorData(const std::string &data);
    void handleDiodeData(const std::string &data);

    void createMenus();
    void createActions();

    QAction *m_quitAction;
    QAction *m_saveAction;
    QAction *m_connectAction;
    QAction *m_disconnectAction;
    QAction *m_sweepTransistorAction;
    QAction *m_sweepDiodeAction;
    QAction *m_persistanceAction;

    std::atomic_bool m_threadRunning;
    float   m_baseCurrent;
    QPointF m_lastCurvePoint;
    
    bool    m_persistance;

    Graph *m_graph;
    QListWidget *m_traceList;

    std::unique_ptr<SerialCtrl> m_serial;
    std::thread m_thread;
};

