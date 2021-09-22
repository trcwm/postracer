#pragma one

#include <thread>
#include <QMainWindow>
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

protected:
    void handleBaseData(const std::string &data);
    void handleCollectorData(const std::string &data);

    void createMenus();
    void createActions();

    QAction *m_quitAction;
    QAction *m_saveAction;
    QAction *m_connectAction;
    QAction *m_disconnectAction;
    QAction *m_sweepTransistorAction;
    QAction *m_sweepDiodeAction;

    float   m_baseCurrent;
    QPointF m_lastCurvePoint;
    
    Graph *m_graph;
    std::unique_ptr<SerialCtrl> m_serial;
    std::thread m_thread;
};

