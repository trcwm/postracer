#include "graph.h"
#include <iostream>
#include <array>
#include <cmath>
#include <algorithm>
#include <QPainter>

#include "tracecolors.h"

Graph::Graph(QWidget *parent) : QWidget(parent)
{
    clearData();

    m_graphMargins.m_left   = 80;
    m_graphMargins.m_right  = 10;
    m_graphMargins.m_top    = 10;
    m_graphMargins.m_bottom = 30;

    QFont font;
    font.setFixedPitch(true);
    setFont(font);

    m_mouseState = MouseState::None;
    m_cursorPos = {0,0};

    m_selectedTrace = -1;

    setMouseTracking(true);
}

void Graph::clearData()
{
    std::unique_lock<std::mutex>(m_mutex);

    m_dataExtents.clear();
    m_traces.clear();
    m_labels.clear();
    m_selectedTrace = -1;
}

size_t Graph::newTrace()
{
    std::unique_lock<std::mutex>(m_mutex);
    m_traces.emplace_back();
    std::cout << "new trace created\n";

    size_t colorIndex = (m_traces.size()-1) % gs_traceColors.size();
    m_traces.back().m_color = gs_traceColors.at(colorIndex);

    return m_traces.size();
}

size_t Graph::getNumberOfTraces() const
{
    std::unique_lock<std::mutex>(m_mutex);
    return m_traces.size();
}


void Graph::addLabel(const QString &txt, const QPointF &p)
{
    m_labels.push_back(LabelType{.m_txt = txt, .m_pos = p});
}

void Graph::addDataPoint(const QPointF &p)
{
    std::unique_lock<std::mutex>(m_mutex);
    if (m_traces.empty())
    {
        m_dataExtents.m_minx = p.x();
        m_dataExtents.m_maxx = p.x();
        m_dataExtents.m_miny = p.y();
        m_dataExtents.m_maxy = p.y();                
        m_traces.emplace_back();
    }

    auto & currentCurve = m_traces.back();

    if (currentCurve.m_data.empty())
    {
        currentCurve.m_data.push_back(p);

        m_dataExtents.m_maxx = std::max(static_cast<float>(p.x()), m_dataExtents.m_maxx);
        m_dataExtents.m_maxy = std::max(static_cast<float>(p.y()), m_dataExtents.m_maxy);
        m_dataExtents.m_miny = std::min(static_cast<float>(p.y()), m_dataExtents.m_miny);  

        m_graphViewport.m_xstart = m_dataExtents.m_minx;
        m_graphViewport.m_ystart = m_dataExtents.m_miny;

        m_graphViewport.m_xspan = m_dataExtents.xspan() * 1.1f;
        m_graphViewport.m_yspan = m_dataExtents.yspan() * 1.1f;

        update();
    }
    else
    {
        currentCurve.m_data.push_back(p);
        m_dataExtents.m_maxx = std::max(static_cast<float>(p.x()), m_dataExtents.m_maxx);
        m_dataExtents.m_maxy = std::max(static_cast<float>(p.y()), m_dataExtents.m_maxy);
        m_dataExtents.m_miny = std::min(static_cast<float>(p.y()), m_dataExtents.m_miny);

        m_graphViewport.m_xstart = m_dataExtents.m_minx;
        m_graphViewport.m_ystart = m_dataExtents.m_miny;

        m_graphViewport.m_xspan = m_dataExtents.xspan() * 1.1f;
        m_graphViewport.m_yspan = m_dataExtents.yspan() * 1.1f;

        update();        
    }
}

void Graph::paintEvent(QPaintEvent *event)
{
    std::unique_lock<std::mutex>(m_mutex);

    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);

    plotAxes(painter);

    size_t colorIndex = 0;
    for(auto const& trace : m_traces)
    {
        if (trace.m_data.size() < 2)
        {
            // don't plot anything because min/max aren't valid
        }
        else
        {
            //float xmul = width() / m_dataExtents.xspan();
            //float ymul = height() / m_dataExtents.yspan();

            painter.setPen(QPen(gs_traceColors.at(colorIndex), 3.0f));

            auto const& firstPoint = trace.m_data.front();
            QPointF firstScreen = graphToScreen(firstPoint.x(), firstPoint.y());
            for(auto const& p : trace.m_data)
            {
                QPointF secondScreen = graphToScreen(p.x(), p.y());
                painter.drawLine(firstScreen, secondScreen);

                firstScreen = secondScreen;
            }
        }

        colorIndex++;
        if (colorIndex >= gs_traceColors.size())
        {
            colorIndex = 0;
        }
    }

    plotLabels(painter);

    if (!m_cursorPos.isNull())
    {
        QPen cursorPen;
        cursorPen.setStyle(Qt::DashDotLine);
        cursorPen.setColor(QColor("#A0FFFFFF"));
        
        auto graphPos = screenToGraph(m_cursorPos);

        auto const& trace = m_traces.at(m_selectedTrace);

        auto iter = std::lower_bound(
            trace.m_data.begin(), 
            trace.m_data.end(), 
            graphPos,           
            [](const QPointF &lhs, const QPointF &rhs) -> bool
                {
                    return lhs.x() < rhs.x();
                }
            );

        if (iter != trace.m_data.end())
        {
            auto nearestPos = graphToScreen(iter->x(), iter->y());
            painter.setPen(cursorPen);
            painter.drawLine(nearestPos.x(), m_graphMargins.m_top, nearestPos.x(), height() - m_graphMargins.m_bottom - 1);
            painter.drawLine(m_graphMargins.m_left, nearestPos.y(), width() - m_graphMargins.m_right - 1, nearestPos.y());

            auto textPos = nearestPos;
            textPos += QPoint(10, -10);

            auto txt = QString::asprintf("%.3f (V), %.2f (mA)", iter->x(), iter->y() * 1000.0f);

            QFontMetrics fontMetrics(font());
            auto textBox = fontMetrics.boundingRect(txt);
            auto textRightPos = textPos.x() + textBox.width();
            auto maxRightPos  = width() - m_graphMargins.m_right - 1;

            if (textRightPos >= maxRightPos)
            {
                // show the text to the left of the cursor
                textPos += QPoint(-textBox.width()-20, 0);
            }

            painter.setPen(QColor("#FFFFFFFF"));            
            painter.drawText(textPos, txt);
            painter.setBrush(QColor("#FFFFFFFF"));
            painter.drawEllipse(nearestPos, 3,3);
        }
    }
}

void Graph::plotLabels(QPainter &painter)
{
    painter.setPen(Qt::white);
    for(auto const& label : m_labels)
    {
        auto pos = graphToScreen(label.m_pos.x(), label.m_pos.y());
        painter.drawText(pos, label.m_txt); 
    }
}

void Graph::plotAxes(QPainter &painter)
{
    float xspan = m_dataExtents.xspan();
    float yspan = m_dataExtents.yspan();

    if ((xspan < 1e-20f) || (yspan < 1e-20f))
    {
        // cannot draw axes -> too small
        return;
    }


    auto r = rect();
    r.moveTo(QPoint(m_graphMargins.m_left, m_graphMargins.m_top));
    r.setSize(QSize(width() - m_graphMargins.m_left - m_graphMargins.m_right, 
        height() - m_graphMargins.m_bottom - m_graphMargins.m_top));
    
    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(r);

    // use between 5 and 10 ticks on every axis
    // with unit steps of 1,2 or 5
    float xunit = pow(10.0f, std::floor(std::log10(xspan)));
    float yunit = pow(10.0f, std::floor(std::log10(yspan)));

    uint32_t xticks = std::ceil(xspan / xunit);
    uint32_t yticks = std::ceil(yspan / yunit);

    if (xticks <= 2)
    {
        xticks *= 5;
        xunit  /= 5.0f;
    }

    if (xticks < 5)
    {
        xticks *= 2;
        xunit  /= 2.0f;
    }

    if (yticks <= 2)
    {
        yticks *= 5;
        yunit  /= 5.0f;
    }

    if (yticks < 5)
    {
        yticks *= 2;
        yunit  /= 2.0f;
    }

    for(uint32_t x=0; x<xticks; x++)
    {   
        auto const pos = graphToScreen(x*xunit, 0);
        painter.setPen(QPen(QColor("#505050"), 2.0f));
        painter.drawLine(pos.x(), m_graphMargins.m_top, pos.x(), height()-1-m_graphMargins.m_bottom);

        painter.setPen(QPen(QColor("#A0A0A0"), 2.0f));
        painter.drawText(QPointF(pos.x(), height()-1), QString::asprintf("%2.1e", xunit*x));
    }

    for(uint32_t y=0; y<yticks; y++)
    {   
        auto const pos = graphToScreen(0,y*yunit);
        painter.setPen(QPen(QColor("#505050"), 2.0f));
        painter.drawLine(m_graphMargins.m_left, pos.y(), width()-1-m_graphMargins.m_right, pos.y());

        painter.setPen(QPen(QColor("#A0A0A0"), 2.0f));
        painter.drawText(QPointF(0, pos.y()), QString::asprintf("%2.1e", yunit*y));
    }
}

QPointF Graph::graphToScreen(float x, float y) const
{
    auto const w = width() - m_graphMargins.m_left - m_graphMargins.m_right;
    auto const h = height() - m_graphMargins.m_top - m_graphMargins.m_bottom;

    float xo = (x-m_graphViewport.m_xstart)/m_graphViewport.m_xspan * w;
    float yo = (y-m_graphViewport.m_ystart)/m_graphViewport.m_yspan * h;

    xo += m_graphMargins.m_left;
    yo += m_graphMargins.m_bottom;

    return QPointF(xo, height() - yo - 1);
}

QPointF Graph::screenToGraphDelta(const QPointF &delta) const
{
    const auto sx = static_cast<float>(m_graphViewport.m_xspan) / width();
    const auto sy = static_cast<float>(m_graphViewport.m_yspan) / height();
    const auto x = (delta.x() * sx);
    const auto y = (delta.y() * sy);

    return QPointF{x,y};
}

QPointF Graph::screenToGraph(const QPointF &p) const
{
    const auto sx = static_cast<float>(m_graphViewport.m_xspan) / width();
    const auto sy = static_cast<float>(m_graphViewport.m_yspan) / height();
    const auto x = (p.x() * sx) + m_graphViewport.m_xstart;
    const auto y = (p.y() * sy) + m_graphViewport.m_ystart;

    return QPointF{x,y};
}

void Graph::mousePressEvent(QMouseEvent *event)
{    
    setCursor(Qt::ClosedHandCursor);
    m_mouseState = MouseState::Dragging;
    m_mouseDownPos = event->pos();
    m_viewportStartDrag = m_graphViewport;
}

void Graph::mouseReleaseEvent(QMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);
    m_mouseState = MouseState::None;
}

void Graph::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseState == MouseState::Dragging)
    {
        auto offset = screenToGraphDelta(m_mouseDownPos - event->pos());
        m_graphViewport = m_viewportStartDrag;
        m_graphViewport.m_xstart += offset.x();
        m_graphViewport.m_ystart -= offset.y();
        update();
    }
    else    
    {
        if ((m_selectedTrace < 0) || (m_selectedTrace >= m_traces.size()))
        {
            if (!m_cursorPos.isNull())
            {
                m_cursorPos = {0,0};
                update();
            }
            return;
        }

        m_cursorPos = event->pos();        
        update();
    }
}

void Graph::wheelEvent(QWheelEvent *event)
{
    //QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    auto mouseChipPos = screenToGraph(event->pos());

    if (numDegrees.y() > 0)
    {
        // limit zoom level to 50
        //m_zoomLevel = std::min(m_zoomLevel+1, 50);
        auto llx = mouseChipPos.x() - ((mouseChipPos.x() - m_graphViewport.m_xstart) * 80 / 100);
        auto lly = mouseChipPos.y() - ((mouseChipPos.y() - m_graphViewport.m_ystart) * 80 / 100);
        auto urx = mouseChipPos.x() + ((m_graphViewport.m_xstart+m_graphViewport.m_xspan - mouseChipPos.x()) * 80 / 100);
        auto ury = mouseChipPos.y() + ((m_graphViewport.m_ystart+m_graphViewport.m_yspan - mouseChipPos.y()) * 80 / 100);

        m_graphViewport.m_xstart = llx;
        m_graphViewport.m_ystart = lly;

        m_graphViewport.m_xspan = urx - llx;
        m_graphViewport.m_yspan = ury - lly;

        update();   
    }
    else if (numDegrees.y() < 0)
    {
        // limit zoom level to 1
        //m_zoomLevel = std::max(m_zoomLevel-1, 1);    

        auto llx = mouseChipPos.x() - ((mouseChipPos.x() - m_graphViewport.m_xstart) * 100 / 80);
        auto lly = mouseChipPos.y() - ((mouseChipPos.y() - m_graphViewport.m_ystart) * 100 / 80);
        auto urx = mouseChipPos.x() + ((m_graphViewport.m_xstart+m_graphViewport.m_xspan - mouseChipPos.x()) * 100 / 80);
        auto ury = mouseChipPos.y() + ((m_graphViewport.m_ystart+m_graphViewport.m_yspan - mouseChipPos.y()) * 100 / 80);

        m_graphViewport.m_xstart = llx;
        m_graphViewport.m_ystart = lly;

        m_graphViewport.m_xspan = urx - llx;
        m_graphViewport.m_yspan = ury - lly;

        update();        
    }    

    event->accept();
}