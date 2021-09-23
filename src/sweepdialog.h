#pragma once
#include <QLineEdit>
#include <QDialog>

#include "customevent.h"
class SweepDialog : public QDialog
{
public:
    SweepDialog(const SweepSetup &setup, QWidget *parent = nullptr);

    auto getSetup() const
    {
        return m_setup;
    }

protected slots:
    void onOk();
    void updateMaxBaseLabel();

protected:
    QLineEdit  *m_baseResistorEdit;
    QLineEdit  *m_baseLimitResistorEdit;
    QLineEdit  *m_collectorResistorEdit;
    QLineEdit  *m_baseStartEdit;
    QLineEdit  *m_baseStopEdit;
    QLineEdit  *m_numSweepsEdit;
    QLabel     *m_maxBaseLabel;

    SweepSetup m_setup;
};