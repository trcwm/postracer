#pragma once

#include <mutex>
#include <vector>
#include <QWidget>
#include <QMouseEvent>

class Graph : public QWidget
{
public:
    Graph(QWidget *parent = 0);

    void clearData();
    void newCurve();

    void addDataPoint(const QPointF &p);
    void addLabel(const QString &txt, const QPointF &p);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void plotAxes(QPainter &painter);
    void plotLabels(QPainter &painter);

    struct LabelType
    {
        QString m_txt;
        QPointF m_pos;
    };

    std::vector<std::vector<QPointF> >  m_curves;
    std::vector<LabelType>              m_labels;
    
    struct ViewPort
    {
        float   m_xspan;
        float   m_yspan;
        float   m_xstart;
        float   m_ystart;
    } m_graphViewport;

    struct
    {
        int32_t m_left;
        int32_t m_right;
        int32_t m_top;
        int32_t m_bottom;
    } m_graphMargins;
    
    struct
    {
        float m_minx;
        float m_maxx;
        float m_miny;
        float m_maxy;

        void clear()
        {
            m_minx = 0;
            m_maxx = 0;
            m_miny = 0;
            m_maxy = 0;
        }

        constexpr float xspan() const
        {
            return m_maxx-m_minx;
        }

        constexpr float yspan() const
        {
            return m_maxy-m_miny;
        }

    } m_dataExtents;

    enum class MouseState
    {
        None,
        Dragging
    } m_mouseState;

    QPointF     graphToScreen(float x, float y) const;
    QPointF     screenToGraphDelta(const QPointF &delta) const;
    QPointF     screenToGraph(const QPointF &p) const;

    QPoint      m_mouseDownPos;
    ViewPort    m_viewportStartDrag;
    std::mutex m_mutex;
};
