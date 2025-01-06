#ifndef SPLASHWINDOW_H
#define SPLASHWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

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
    QLabel *logoLabel;
    QTimer *timer;
};

#endif // SPLASHWINDOW_H 