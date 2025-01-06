#include "splashwindow.h"
#include <QPainter>
#include <QVBoxLayout>

SplashWindow::SplashWindow(const QSize &size, const QPoint &pos, QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // 设置大小和位置
    resize(size);
    move(pos);
    
    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建并设置 logo
    logoLabel = new QLabel(this);
    QPixmap logo(":/images/logo.png");
    logoLabel->setPixmap(logo.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(logoLabel);
    
    // 创建定时器
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this]() {
        emit finished();
        close();
    });
    
    // 启动定时器
    timer->start(1000);  // 1秒
}

void SplashWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制半透明背景
    painter.fillRect(rect(), QColor(255, 255, 255, 240));
    
    QWidget::paintEvent(event);
} 