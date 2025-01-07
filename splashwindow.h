#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class SplashWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SplashWindow(const QSize &size, const QPoint &pos, QWidget *parent = nullptr);

signals:
    void finished();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void startFadeOut();

private:
    QLabel *logoLabel;
    QTimer *timer;
    QGraphicsOpacityEffect *opacityEffect;
    QPropertyAnimation *fadeAnimation;
};

#endif // SPLASHWINDOW_H 