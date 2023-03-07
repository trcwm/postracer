#include "graph.h"
#include <iostream>
#include <array>
#include <cmath>
#include <algorithm>
#include <QPainter>
#include <QDebug>

#include "tracecolors.h"

PlotRect::PlotRect()
{
    m_plotRect = {0,0,0,0};
}

void PlotRect::setDataRect(const QRectF &dataRect)
{
    m_dataRect = dataRect;
}

QRectF PlotRect::getDataRect() const
{
    return m_dataRect;
}

void PlotRect::setPlotRect(const QRect &plotRect)
{
    m_plotRect = plotRect;
}

void PlotRect::plotData(QPainter &painter, 
    const std::vector<QPointF> &data, const QColor &lineColor)
{
    if (data.size() <= 1)
    {
        return;
    }

    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(QPen(lineColor, 3.0f));

    painter.setClipRect(m_plotRect);
    painter.setClipping(true);

    auto lineStart = graphToScreen(data.front());
    for(const auto& p : data)
    {
        auto lineEnd = graphToScreen(p);

        painter.drawLine(lineStart, lineEnd);
        lineStart = lineEnd;
    }

    painter.setClipping(false);
}

void PlotRect::clearRect(QPainter &painter)
{
    painter.fillRect(m_plotRect,Qt::black);
}

void PlotRect::drawOutline(QPainter &painter)
{
    painter.setRenderHints(QPainter::Antialiasing, false /* no anti-aliasing */);
    //painter.setRenderHints(QPainter::HighQualityAntialiasing, false /* no anti-aliasing */);
    painter.setPen(QColor("#FFFFFF"));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_plotRect);
}

QPointF PlotRect::graphToScreen(const QPointF &p) const
{
    const float xmul = static_cast<float>(m_plotRect.width()) / m_dataRect.width();
    const float ymul = static_cast<float>(m_plotRect.height()) / m_dataRect.height();
    float x = m_plotRect.left() + (p.x() - m_dataRect.left()) * xmul; 
    float y = m_plotRect.bottom() - (p.y() - m_dataRect.top()) * ymul;
    return QPointF{x,y};
}

QPointF PlotRect::screenToGraph(const QPointF &p) const
{
    const float xmul = static_cast<float>(m_plotRect.width()) / m_dataRect.width();
    const float ymul = static_cast<float>(m_plotRect.height()) / m_dataRect.height();

    float x = (p.x() - m_plotRect.left()) / xmul + m_dataRect.left();
    float y = (p.y() + m_plotRect.bottom()) / ymul + m_dataRect.top();

    return QPointF{x,y};    
}

Graph::Graph(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

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
    m_traces.back().m_visible = true;

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

void Graph::addLabel(const QString &txt)
{
    if (m_traces.size() == 0) return;

    auto p = m_traces.back().m_data.back();
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
        m_dataExtents.m_minx = std::min(static_cast<float>(p.x()), m_dataExtents.m_minx);  
        m_dataExtents.m_miny = std::min(static_cast<float>(p.y()), m_dataExtents.m_miny);  

        m_plotRect.setDataRect(
            QRectF{
                m_dataExtents.m_minx, m_dataExtents.m_miny,
                (m_dataExtents.m_maxx - m_dataExtents.m_minx) * 1.1f,
                (m_dataExtents.m_maxy - m_dataExtents.m_miny) * 1.1f
            });

        update();
    }
    else
    {
        currentCurve.m_data.push_back(p);
        m_dataExtents.m_maxx = std::max(static_cast<float>(p.x()), m_dataExtents.m_maxx);
        m_dataExtents.m_maxy = std::max(static_cast<float>(p.y()), m_dataExtents.m_maxy);
        m_dataExtents.m_minx = std::min(static_cast<float>(p.x()), m_dataExtents.m_minx);
        m_dataExtents.m_miny = std::min(static_cast<float>(p.y()), m_dataExtents.m_miny);

        m_plotRect.setDataRect(
            QRectF{
                m_dataExtents.m_minx, m_dataExtents.m_miny,
                (m_dataExtents.m_maxx - m_dataExtents.m_minx) * 1.1f,
                (m_dataExtents.m_maxy - m_dataExtents.m_miny) * 1.1f
            });

        update();        
    }
}

void Graph::resizeEvent(QResizeEvent *event)
{
    QSize newSize = event->size();
    updatePlotRectSize();
}

void Graph::updatePlotRectSize()
{
    m_plotRect.setPlotRect(
        QRect{
            m_graphMargins.m_left, 
            m_graphMargins.m_top,
            size().width()-m_graphMargins.m_left-m_graphMargins.m_right, 
            size().height()-m_graphMargins.m_top-m_graphMargins.m_bottom
    });
}

void Graph::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    painter.setRenderHint(QPainter::Antialiasing);

    m_plotRect.clearRect(painter);
    
    // plot axes
    plotAxes(painter);

    // plot traces
    for(auto const& trace : m_traces)
    {
        if (trace.m_visible)
        {
            m_plotRect.plotData(painter, trace.m_data, trace.m_color);
        }
    }

    m_plotRect.drawOutline(painter);
    
    plotLabels(painter);

    drawCursor(painter);
    drawMarkers(painter);
}

void Graph::drawCursor(QPainter &painter)
{
    if ((!m_cursorPos.isNull()) && (m_selectedTrace >=0) && (m_selectedTrace < m_traces.size()))
    {
        QPen cursorPen;
        cursorPen.setStyle(Qt::DashDotLine);
        cursorPen.setColor(QColor("#FFFFFF"));
        
        auto graphPos = m_plotRect.screenToGraph(m_cursorPos);

        auto const& trace = m_traces.at(m_selectedTrace);

        if (!trace.m_data.empty())
        {
            auto iter = std::lower_bound(
                trace.m_data.begin(), 
                trace.m_data.end(), 
                graphPos,           
                [](const QPointF &lhs, const QPointF &rhs) -> bool
                    {
                        return lhs.x() < rhs.x();
                    }
                );

            // if we're past the end of the data array
            // take the last point
            if (iter == trace.m_data.end())
            {
                iter = trace.m_data.begin() + trace.m_data.size()-1;
            }

            m_lastCursorPos = QPointF{iter->x(), iter->y()};
            m_lastCursorPosValid = true;

            auto nearestPos = m_plotRect.graphToScreen(QPointF{iter->x(), iter->y()});
            painter.setPen(cursorPen);
            painter.drawLine(nearestPos.x(), m_graphMargins.m_top, nearestPos.x(), height() - m_graphMargins.m_bottom - 1);
            painter.drawLine(m_graphMargins.m_left, nearestPos.y(), width() - m_graphMargins.m_right - 1, nearestPos.y());

            auto textPos = nearestPos;
            textPos += QPoint(10, -10);

            auto txt = QString::asprintf("%g (%s), %g (%s)", iter->x(), m_xUnitStr.toStdString().c_str(), 
                iter->y(), m_yUnitStr.toStdString().c_str());

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
    QFontMetrics fm(font());

    painter.setPen(Qt::white);
    painter.setBrush(Qt::black);
    painter.setRenderHints(QPainter::RenderHint::Antialiasing);
    for(auto const& label : m_labels)
    {
        auto bb = fm.boundingRect(label.m_txt);
        auto pos = m_plotRect.graphToScreen( QPointF{label.m_pos.x(), label.m_pos.y()} );
        pos += QPointF{8,0};

        painter.drawRoundedRect(bb.adjusted(pos.x()-4, pos.y()-4, pos.x()+4, pos.y()+4), 4, 4);
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

    QFontMetrics fm(font());

    for(uint32_t x=0; x<xticks; x++)
    {   
        auto const pos = m_plotRect.graphToScreen( QPointF{x*xunit, 0} );

        // dont plot ticks that in the left margin
        if (pos.x() < m_plotRect.left())
        {
            continue;
        }

        // dont plot ticks that in the right margin
        if (pos.x() >= m_plotRect.right())
        {
            continue;
        }

        painter.setPen(QPen(QColor("#505050"), 2.0f, Qt::PenStyle::DashDotDotLine));
        painter.drawLine(pos.x(), m_graphMargins.m_top, pos.x(), height()-m_graphMargins.m_bottom);

        const auto txt = QString::asprintf("%2.1e", xunit*x);
        auto bb  = fm.boundingRect(txt);
        auto txtpos = QPointF(QPointF(pos.x(), height()-1));
        txtpos += QPointF{- bb.width() / 2.0f, (bb.height() / 2.0f) - m_graphMargins.m_bottom/2.0f};

        painter.setPen(QPen(QColor("#A0A0A0"), 2.0f));
        painter.drawText(txtpos, txt);
    }

    for(uint32_t y=0; y<yticks; y++)
    {   
        auto const pos = m_plotRect.graphToScreen( QPointF{0,y*yunit} );

        // dont plot ticks that in the top margin
        if (pos.y() < m_plotRect.top())
        {
            continue;
        }

        painter.setPen(QPen(QColor("#505050"), 2.0f, Qt::PenStyle::DashDotDotLine));
        painter.drawLine(m_graphMargins.m_left, pos.y(), width()-1-m_graphMargins.m_right, pos.y());

        const auto txt = QString::asprintf("%2.1e", yunit*y);
        auto bb  = fm.boundingRect(txt);
        auto txtpos = QPointF(0, pos.y());
        txtpos += QPointF{m_graphMargins.m_left / 2.0f - bb.width() / 2.0f, 0};

        painter.setPen(QPen(QColor("#A0A0A0"), 2.0f));
        painter.drawText(txtpos, txt);
    }
}

void Graph::drawMarkers(QPainter &painter)
{
    if (m_markerA.m_valid)
    {
        auto mAPos = m_plotRect.graphToScreen(m_markerA.m_point);
        painter.setPen(Qt::white);
        painter.drawEllipse(mAPos, 4,4);
    }
    if (m_markerB.m_valid)
    {
        auto mBPos = m_plotRect.graphToScreen(m_markerB.m_point);
        
        painter.setPen(QPen(Qt::white, 1));
        painter.drawEllipse(mBPos, 4,4);
    }
    
    if ((m_markerA.m_valid) && (m_markerB.m_valid))
    {
        auto p1 = m_markerA.m_point;
        auto p2 = m_markerB.m_point;
        if (p1.x() > p2.x())
        {
            std::swap(p1,p2);
        }

        float dx = p2.x() - p1.x();
        float dy = p2.y() - p1.y();

        if (dx < 1e-6f) return;    // x distance too small for drawing a line?

        // calculate x intercept point
        float c = dy/dx;
        float offset = p1.y() - c*p1.x();

        float x_intercept = -offset / c;

        QString txt = "X intercept = ";
        txt.append(QString::number(x_intercept, 'g',3));

        auto mAPos = m_plotRect.graphToScreen(m_markerA.m_point);
        auto mBPos = m_plotRect.graphToScreen(m_markerB.m_point);

        // make sure the text is positioned 
        // above the line
        QFontMetricsF fm(font());

        QPointF txtpos = mAPos;        
        if (txtpos.y() > mBPos.y())
        {
            txtpos = mBPos;
        }
        
        txtpos.ry() -= fm.height();
        painter.drawText(txtpos, txt);

        txt = "Slope = ";
        txt.append(QString::number(c, 'g',3));
        txtpos.ry() -= fm.height();
        painter.drawText(txtpos, txt);

        painter.setPen(QPen(Qt::white, 3));
        painter.drawLine(mAPos, mBPos);
    }
}

#if 0

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
#endif


void Graph::mousePressEvent(QMouseEvent *event)
{    
    setCursor(Qt::ClosedHandCursor);
    m_mouseState = MouseState::Dragging;
    m_mouseDownPos = event->pos();
    m_dataRectStartDrag = m_plotRect.getDataRect();
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
        auto offset = m_plotRect.screenToGraph(m_mouseDownPos) - m_plotRect.screenToGraph(event->pos());
        auto newDataRect = m_dataRectStartDrag;
        newDataRect.adjust(offset.x(), -offset.y(), offset.x(), -offset.y());
        m_plotRect.setDataRect(newDataRect);
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

    //auto mouseChipPos = screenToGraph(event->pos());

    if (numDegrees.y() > 0)
    {
        // limit zoom level to 50
        //m_zoomLevel = std::min(m_zoomLevel+1, 50);
        //auto llx = mouseChipPos.x() - ((mouseChipPos.x() - m_graphViewport.m_xstart) * 80 / 100);
        //auto lly = mouseChipPos.y() - ((mouseChipPos.y() - m_graphViewport.m_ystart) * 80 / 100);
        //auto urx = mouseChipPos.x() + ((m_graphViewport.m_xstart+m_graphViewport.m_xspan - mouseChipPos.x()) * 80 / 100);
        //auto ury = mouseChipPos.y() + ((m_graphViewport.m_ystart+m_graphViewport.m_yspan - mouseChipPos.y()) * 80 / 100);

        //m_graphViewport.m_xstart = llx;
        //m_graphViewport.m_ystart = lly;

        //m_graphViewport.m_xspan = urx - llx;
        //m_graphViewport.m_yspan = ury - lly;

        update();   
    }
    else if (numDegrees.y() < 0)
    {
        // limit zoom level to 1
        //m_zoomLevel = std::max(m_zoomLevel-1, 1);    

        //auto llx = mouseChipPos.x() - ((mouseChipPos.x() - m_graphViewport.m_xstart) * 100 / 80);
        //auto lly = mouseChipPos.y() - ((mouseChipPos.y() - m_graphViewport.m_ystart) * 100 / 80);
        //auto urx = mouseChipPos.x() + ((m_graphViewport.m_xstart+m_graphViewport.m_xspan - mouseChipPos.x()) * 100 / 80);
        //auto ury = mouseChipPos.y() + ((m_graphViewport.m_ystart+m_graphViewport.m_yspan - mouseChipPos.y()) * 100 / 80);

        //m_graphViewport.m_xstart = llx;
        //m_graphViewport.m_ystart = lly;

        //m_graphViewport.m_xspan = urx - llx;
        //m_graphViewport.m_yspan = ury - lly;

        update();        
    }    

    event->accept();
}

void Graph::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_A)
    {
        qDebug() << "A";
        if (m_lastCursorPosValid)
        {
            m_markerA.m_point = m_lastCursorPos;
            m_markerA.m_valid = true;
            update();
        }
    }    
    else if (event->key()==Qt::Key_B)
    {
        if (m_lastCursorPosValid)
        {
            m_markerB.m_point = m_lastCursorPos;
            m_markerB.m_valid = true;
            update();
        }
    }        
    else if (event->key()==Qt::Key_C)
    {
        m_markerA.m_valid = false;
        m_markerB.m_valid = false;
    }            
}
