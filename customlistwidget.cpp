/*
 * Copyright (c) 2024 FilmTool
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "customlistwidget.h"
#include <QScrollBar>
#include <QDebug>

CustomListWidget::CustomListWidget(QWidget *parent)
    : QListWidget(parent)
    , dropIndicatorRect()
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    // 设置统一的项目大小
    setUniformItemSizes(true);
    // 设置自动调整大小
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    // 设置调整模式
    setResizeMode(QListView::Adjust);
    
    // 使用样式表设置内边距
    setStyleSheet("QListWidget { padding: 10px; }");
}

void CustomListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QListWidget::dragMoveEvent(event);
    
    // 获取当前拖放位置的项目
    QListWidgetItem *currentItem = itemAt(event->position().toPoint());
    if (currentItem) {
        QRect rect = visualItemRect(currentItem);
        QPoint pos = event->position().toPoint();
        
        // 根据鼠标位置决定是在项目上方还是下方显示指示器
        if (pos.y() < rect.center().y()) {
            // 在当前项目上方
            dropIndicatorRect = QRect(rect.left(), rect.top() - 2, rect.width(), 4);
        } else {
            // 在当前项目下方
            dropIndicatorRect = QRect(rect.left(), rect.bottom() - 2, rect.width(), 4);
        }
    } else if (count() > 0) {
        // 如果拖到空白区域，则在最后一个项目下方
        QListWidgetItem *lastItem = item(count() - 1);
        if (lastItem) {
            QRect rect = visualItemRect(lastItem);
            dropIndicatorRect = QRect(rect.left(), rect.bottom() - 2, rect.width(), 4);
        }
    }
    
    viewport()->update();
    event->accept();
}

void CustomListWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    dropIndicatorRect = QRect();
    viewport()->update();
    QListWidget::dragLeaveEvent(event);
}

void CustomListWidget::dropEvent(QDropEvent *event)
{
    dropIndicatorRect = QRect();
    QListWidget::dropEvent(event);
    viewport()->update();
}

void CustomListWidget::paintEvent(QPaintEvent *event)
{
    QListWidget::paintEvent(event);
    
    if (!dropIndicatorRect.isNull()) {
        QPainter painter(viewport());
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 120, 215));
        painter.drawRect(dropIndicatorRect);
    }
}
