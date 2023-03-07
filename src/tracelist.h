#pragma once

#include <QListWidget>

class TraceList : public QListWidget
{
public:
    TraceList(QWidget *parent = nullptr);

protected:
    void keyDownEvent(QKeyEvent *key) override;
};