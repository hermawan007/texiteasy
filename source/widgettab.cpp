#include "widgettab.h"
#include <QPainter>
#include <QMouseEvent>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QFontMetrics>
#include "configmanager.h"
#include "widgettextedit.h"
#include "widgetfile.h"

WidgetTab::WidgetTab(QWidget *parent) :
    QWidget(parent)
{
    _currentIndex = -1;
    _widgetOverId = -1;
    this->setMinimumHeight(30);
    this->setMaximumHeight(30);
    this->setMouseTracking(true);

    _overCloseId = -1;

    _padding = 10;
    _margin = 5;
    _closeLeftMargin = 7;
    _moreRightMargin = 7;
    _closeWidth = 5;
    _moreWidth = 5;

    this->setContextMenuPolicy(Qt::NoContextMenu);
    this->addAction(new QAction("tesssst", this));
}

void WidgetTab::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    QFontMetrics fm(painter.font());

    bool darkTheme = ConfigManager::Instance.getTextCharFormats("line-number").background().color().value() < 150;

    QPen defaultPen(darkTheme ? ConfigManager::Instance.getTextCharFormats("line-number").foreground().color().darker(150) :
                                ConfigManager::Instance.getTextCharFormats("line-number").foreground().color().lighter(160));
    QPen hoverPen(darkTheme ? ConfigManager::Instance.getTextCharFormats("line-number").foreground().color() :
                              ConfigManager::Instance.getTextCharFormats("line-number").foreground().color().darker(150));
    QPen defaultClosePen(QColor(100,100,100));
    defaultClosePen.setWidth(2);
    QPen hoverClosePen(QColor(130,130,130));
    hoverClosePen.setWidth(2);
    QPen overClosePen(QColor(160,160,160));
    overClosePen.setWidth(2);

    QPen rectPen(QColor(0,0,0));
    QBrush backgroundBrush(ConfigManager::Instance.getTextCharFormats("line-number").background().color().darker(200));
    QBrush defaultRectBrush(darkTheme ?
                                ConfigManager::Instance.getTextCharFormats("line-number").background().color().lighter(130) :
                                ConfigManager::Instance.getTextCharFormats("line-number").background().color().darker(160));
    QBrush hoverRectBrush(ConfigManager::Instance.getTextCharFormats("line-number").background().color());


    painter.setBrush(backgroundBrush);
    painter.drawRect(-1, -1, width() + 2, height() + 2);

    int index = 0;
    int cummulatedWidth = 10;
    painter.translate(10,0);
    _tabsNameWidth.clear();
    _tabsNameWidth.append(cummulatedWidth);
    int xBegin, yBegin, xEnd, yEnd;

    foreach(QString tabName, _tabsName)
    {
        painter.transform().map(0, 0, &xBegin, &yBegin);
        if(index == this->currentIndex())
        {
            painter.setBrush(hoverRectBrush);
        }
        else
        {
            painter.setBrush(defaultRectBrush);
        }
        painter.setPen(rectPen);
        painter.drawRoundedRect(0, 5, tabWidth(index, painter.font()), 30, 5, 5);
        painter.translate(_padding, 0);
        this->drawMoreButton(&painter, index);

        if(index == this->currentIndex())
        {
            painter.setPen(hoverPen);
        }
        else
        {
            painter.setPen(defaultPen);
        }
        painter.drawText(0, 22, tabName);
        painter.translate(fm.width(tabName),0);

        if(index == _overCloseId)
        {
            painter.setPen(overClosePen);
        }
        else
        if(index == this->currentIndex())
        {
            painter.setPen(hoverClosePen);
        }
        else
        {
            painter.setPen(defaultClosePen);
        }
        painter.translate(_closeLeftMargin, 0);
        painter.drawLine(0, 15, _closeWidth, 15 + _closeWidth);
        painter.drawLine(0, 15 + _closeWidth, _closeWidth, 15);
        painter.translate(_closeWidth + _padding, 0);

        painter.translate(_margin, 0);


        painter.transform().map(0, 0, &xEnd, &yEnd);

        cummulatedWidth += (xEnd - xBegin);
        _tabsNameWidth.append(cummulatedWidth);
        ++index;
    }


}
int WidgetTab::tabWidth(int index, const QFont &font)
{
    QFontMetrics fm(font);
    int width = fm.width(_tabsName.at(index)) + _padding * 2 + _closeLeftMargin + _closeWidth;
    if(this->widget(index)->actions().isEmpty())
    {
        return width;
    }
    return width + _moreWidth + _moreRightMargin;
}

void WidgetTab::drawMoreButton(QPainter *painter, int index)
{
    if(this->widget(index)->actions().isEmpty())
    {
        return;
    }
    QBrush defaultMoreBrush(QColor(100,100,100));
    QBrush hoverMoreBrush(QColor(130,130,130));
    QBrush overMoreBrush(QColor(160,160,160));

    painter->setPen(Qt::NoPen);
    if(index == _overMoreId)
    {
        painter->setBrush(overMoreBrush);
    }
    else
    if(index == this->currentIndex())
    {
        painter->setBrush(hoverMoreBrush);
    }
    else
    {
        painter->setBrush(defaultMoreBrush);
    }
    QVector<QPointF> pol;
    pol.append(QPointF(0, 15));
    pol.append(QPointF(qreal(_moreWidth)/2.0, 20));
    pol.append(QPointF(_moreWidth, 15));
    QPolygonF polF(pol);
    painter->drawConvexPolygon(polF);
    painter->translate(_moreWidth + _moreRightMargin, 0);
}

void WidgetTab::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::RightButton)
    {
        return;
    }
    _pressCloseId = _pressMoreId = false;
    int idx = -1;
    int lastWidth = 0;
    foreach(int w, _tabsNameWidth)
    {
        if(idx == -1 && event->pos().x() < w)
        {
            return;
        }
        if(event->pos().x() < w)
        {
            if(!this->widget(idx)->actions().isEmpty() && this->overMoreButton(event->pos(), lastWidth))
            {
                _pressMoreId = idx;
                return;
            }
            if(this->overCloseButton(event->pos(), w))
            {
                _pressCloseId = idx;
                return;
            }
            return;
        }
        lastWidth = w;
        ++idx;
    }
    return;
}

void WidgetTab::mouseReleaseEvent(QMouseEvent * event)
{
    if(event->button() == Qt::RightButton)
    {
        return;
    }
    int idx = -1;
    int lastWidth = 0;
    foreach(int w, _tabsNameWidth)
    {
        if(idx == -1 && event->pos().x() < w)
        {
            return;
        }
        if(event->pos().x() < w)
        {
            if(!this->widget(idx)->actions().isEmpty() && this->overMoreButton(event->pos(), lastWidth))
            {
                _overMoreId = idx;
                this->contextMenuEvent(new QContextMenuEvent(QContextMenuEvent::Mouse,event->pos()));
                return;
            }
            if(this->overCloseButton(event->pos(), w) && _pressCloseId == idx)
            {
                _overCloseId = idx;
                emit tabCloseRequested(_overCloseId);
                update();
                return;
            }
            setCurrentIndex(idx);
            return;
        }
        lastWidth = w;
        ++idx;
    }
    return;
}

bool WidgetTab::overCloseButton(QPoint mousePos, int left)
{
    return mousePos.x() <= left - _padding - _margin + 2 &&
           mousePos.x() >= left - _padding - _margin - _closeWidth - 2 &&
           mousePos.y() >= 15 - 2 &&
           mousePos.y() <= 15 + _closeWidth + 2;
}
bool WidgetTab::overMoreButton(QPoint mousePos, int left)
{
    return mousePos.x() <= left + _padding + _moreWidth + 2 &&
           mousePos.x() >= left + _padding - 2 &&
           mousePos.y() >= 15 - 2 &&
           mousePos.y() <= 15 + 5 + 2;
}

void WidgetTab::mouseMoveEvent(QMouseEvent * event)
{
    _overCloseId = -1;
    _overMoreId = -1;
    this->setCursor(Qt::ArrowCursor);
    int idx = -1;
    int lastWidth = 0;
    foreach(int w, _tabsNameWidth)
    {
        if(idx == -1 && event->pos().x() < w)
        {
            break;
        }
        if(event->pos().x() < w)
        {
            if(!this->widget(idx)->actions().isEmpty() && this->overMoreButton(event->pos(), lastWidth))
            {
                this->setCursor(Qt::PointingHandCursor);
                _overMoreId = idx;
                update();
                return;
            }
            if(this->overCloseButton(event->pos(), w))
            {
                this->setCursor(Qt::PointingHandCursor);
                _overCloseId = idx;
                update();
                return;
            }
            update();
            return;
        }
        lastWidth = w;
        ++idx;
    }
}
void WidgetTab::contextMenuEvent(QContextMenuEvent * event)
{
    if(_overMoreId == -1)
    {
        return;
    }
    QPoint p(_tabsNameWidth.at(_overMoreId), 30);
    QMenu * m = new QMenu(this);
    m->addActions(this->widget(_overMoreId)->actions());
    _overMoreId = -1;
    m->exec(this->mapToGlobal(p));
    delete m;
}

WidgetFile * WidgetTab::widget(QString filename)
{
    foreach(WidgetFile * widget, _widgets)
    {
        if(!filename.compare(widget->widgetTextEdit()->getCurrentFile()->getFilename()))
        {
            return widget;
        }
    }
    return 0;
}
int WidgetTab::indexOf(QString filename)
{
    int index = 0;
    foreach(WidgetFile * widget, _widgets)
    {
        if(!filename.compare(widget->widgetTextEdit()->getCurrentFile()->getFilename()))
        {
            return index;
        }
        ++index;
    }
    return -1;
}

void WidgetTab::removeAll()
{
    foreach(WidgetFile * widget, _widgets)
    {
        delete widget;
    }
    _widgets.clear();
    _tabsName.clear();
    this->setCurrentIndex(-1);
}
void WidgetTab::removeTab(WidgetFile *widget)
{
    int index = _widgets.indexOf(widget);
    this->removeTab(index);
}
void WidgetTab::removeTab(int index)
{
    _widgets.removeAt(index);
    _tabsName.removeAt(index);
    if(this->currentIndex() < index)
    {
        update();
        return;
    }
    if(this->currentIndex() > index)
    {
        this->setCurrentIndex(this->currentIndex() - 1);
        return;
    }
    if(!this->count())
    {
        this->setCurrentIndex(-1);
        return;
    }
    if(index == 0)
    {
        emit currentChanged(_widgets.at(index));
        return;
    }
    this->setCurrentIndex(index - 1);
}

void WidgetTab::initTheme()
{
}
void WidgetTab::mouseDoubleClickEvent(QMouseEvent * event)
{
    if(_tabsName.empty() || event->pos().x() > _tabsNameWidth.last())
    {
        emit newTabRequested();
    }
}
