#pragma once
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QWidget>
#include <QTabWidget>

#include "messagetypes.h"

namespace SweepPage
{

class Base : public QWidget
{
    Q_OBJECT
public:
    Base(QWidget *parent = nullptr);

    void populate(const Messages::SweepBase &base);

    Messages::SweepBase get() const noexcept;

protected:
    QLineEdit  *m_baseStartEdit;
    QLineEdit  *m_baseStopEdit;
    QLineEdit  *m_collectorVoltageEdit;
};

class Collector : public QWidget
{
    Q_OBJECT
public:
    Collector(QWidget *parent = nullptr);

    void populate(const Messages::SweepCollector &collector);

    Messages::SweepCollector get() const noexcept;

protected:
    QLineEdit  *m_collectorStartEdit;
    QLineEdit  *m_collectorStopEdit;
    QLineEdit  *m_baseCurrentEdit;
};

class Diode : public QWidget
{
    Q_OBJECT
public:
    Diode(QWidget *parent = nullptr);

    void populate(const Messages::SweepDiode &diode);

    Messages::SweepDiode get() const noexcept;

protected:
    QLineEdit  *m_currentStartEdit;
    QLineEdit  *m_currentStopEdit;
};

};


class SweepDialog : public QDialog
{
    Q_OBJECT
public:
    SweepDialog(const Messages::SweepSetup &setup, QWidget *parent = nullptr);

    auto getSetup() const
    {
        return m_setup;
    }

protected slots:
    void onOk();

protected:
    SweepPage::Base         *m_baseSweep = nullptr;
    SweepPage::Collector    *m_collectorSweep = nullptr;
    SweepPage::Diode        *m_diodeSweep = nullptr;

    QTabWidget *m_tabs;
    QLineEdit  *m_numPointsEdit;

    Messages::SweepSetup    m_setup;
};
