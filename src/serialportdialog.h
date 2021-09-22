#pragma once
#include <QDialog>
#include <QListWidget>

class SerialPortDialog : public QDialog
{
public:
    SerialPortDialog(QWidget *parent = nullptr);

    QString getSerialPortLocation() const;

protected:
    QListWidget *m_portList;
};