#pragma once

#include <mutex>
#include <vector>
#include <QWidget>
#include <QMouseEvent>

/** helper class that plots a data traces */
class PlotRect
{
public:
    PlotRect();

    QRectF getDataRect() const;
    void setDataRect(const QRectF &dataRect);
    void setPlotRect(const QRect &plotRect);

    void clearRect(QPainter &painter);
    void drawOutline(QPainter &painter);

    void plotData(QPainter &painter,
        const std::vector<QPointF> &data, 
        const QColor &lineColor);

    QPointF graphToScreen(const QPointF &p) const;
    QPointF screenToGraph(const QPointF &p) const;

    constexpr auto top() const
    {
        return m_plotRect.top();
    }

    constexpr auto bottom() const
    {
        return m_plotRect.bottom();
    }

    constexpr auto left() const
    {
        return m_plotRect.left();
    }

    constexpr auto right() const
    {
        return m_plotRect.right();
    }

protected:

    QRectF  m_dataRect;
    QRect   m_plotRect;
};


class Graph : public QWidget
{
public:
    Graph(QWidget *parent = 0);

    void clearData();
    
    /** creates a new trace and return the total number of traces */
    size_t newTrace();

    void addDataPoint(const QPointF &p);
    void addLabel(const QString &txt, const QPointF &p);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void selectTrace(int32_t trace)
    {
        m_selectedTrace = trace;
        update();
    }

    void setUnitStrings(QString xUnitStr, QString yUnitStr)
    {
        m_xUnitStr = xUnitStr;
        m_yUnitStr = yUnitStr;
    }

    /** get number of traces - thread safe */
    size_t getNumberOfTraces() const;

    /** direct access to traces - not thread safe */
    auto const& traces() const
    {
        return m_traces;
    }

protected:
    void paintEvent(QPaintEvent *event) override;

    void plotAxes(QPainter &painter);
    void plotLabels(QPainter &painter);
    void drawMarker(QPainter &painter);
    void updatePlotRectSize();

    struct LabelType
    {
        QString m_txt;
        QPointF m_pos;
    };

    QString m_xUnitStr;
    QString m_yUnitStr;

    struct TraceType
    {
        std::vector<QPointF> m_data;
        QColor               m_color;
        bool                 m_visible;
    };

    std::vector<TraceType> m_traces;
    std::vector<LabelType> m_labels;
    
    PlotRect m_plotRect;

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

    int32_t     m_selectedTrace;

    QPoint      m_mouseDownPos;
    QPoint      m_cursorPos;

    QRectF      m_dataRectStartDrag;
    std::mutex m_mutex;
};
