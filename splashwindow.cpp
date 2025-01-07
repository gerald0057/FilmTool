#include "splashwindow.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QApplication>

SplashWindow::SplashWindow(const QSize &size, const QPoint &pos, QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 获取窗口框架尺寸
    int titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
    int frameWidth = QApplication::style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    // 计算完整尺寸（包括边框）
    QSize fullSize(
        size.width() + frameWidth * 2,
        size.height() + titleBarHeight + frameWidth
    );

    // 计算偏移位置（向左上偏移以覆盖边框）
    QPoint fullPos(
        pos.x() - frameWidth,
        pos.y() - frameWidth
    );

    // 设置大小和位置
    resize(fullSize);
    move(fullPos);

    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建并设置 logo
    logoLabel = new QLabel(this);
    QPixmap logo(":/images/logo.png");
    logoLabel->setPixmap(logo.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(logoLabel);

    // 创建并设置不透明度效果
    opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(1.0);
    setGraphicsEffect(opacityEffect);

    // 创建淡出动画
    fadeAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeAnimation->setDuration(500);  // 500ms的淡出时间
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    fadeAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(fadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        emit finished();
        close();
    });

    // 创建定时器
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &SplashWindow::startFadeOut);

    // 启动定时器
    timer->start(2000);
}

void SplashWindow::startFadeOut()
{
    fadeAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SplashWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), QColor(255, 255, 255, 255));
    
    QWidget::paintEvent(event);
} 