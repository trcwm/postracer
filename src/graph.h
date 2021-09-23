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
    
    /** creates a new trace and return the total number of traces */
    size_t newTrace();

    void addDataPoint(const QPointF &p);
    void addLabel(const QString &txt, const QPointF &p);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void selectTrace(int32_t trace)
    {
        m_selectedTrace = trace;
        update();
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

    struct LabelType
    {
        QString m_txt;
        QPointF m_pos;
    };

    struct TraceType
    {
        std::vector<QPointF> m_data;
        QColor               m_color;
    };

    std::vector<TraceType> m_traces;
    std::vector<LabelType> m_labels;
    
    struct ViewPort
    {
        float   m_xspan;
        float   m_yspan;
        float   m_xstart;
        float   m_ystart;

        constexpr float left() const noexcept
        {
            return m_xstart;
        }

        constexpr float right() const noexcept
        {
            return m_xstart + m_xspan;
        }        

        constexpr float top() const noexcept
        {
            return m_ystart;
        }

        constexpr float bottom() const noexcept
        {
            return m_ystart + m_yspan;
        }        

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

    int32_t     m_selectedTrace;

    QPoint      m_mouseDownPos;
    QPoint      m_cursorPos;

    ViewPort    m_viewportStartDrag;
    std::mutex m_mutex;
};
