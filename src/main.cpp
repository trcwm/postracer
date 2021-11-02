#include <QApplication>
#include <QDesktopWidget>
#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

    MainWindow window(nullptr);

    window.show();

    window.setMinimumSize(720, 405);
    window.setWindowTitle("POSTracer");
    
    int width = window.frameGeometry().width();
    int height = window.frameGeometry().height();
    
    QDesktopWidget wid;
    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();
    window.setGeometry((screenWidth/2)-(width/2),(screenHeight/2)-(height/2),width,height);

    return app.exec();
}
