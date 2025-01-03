#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QPushButton>
#include <QVideoWidget>
#include <QSlider>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openVideo();
    void playPauseVideo();
    void updateDuration(qint64 duration);
    void updatePosition(qint64 position);
    void setPosition(int position);

private:
    Ui::MainWindow *ui;
    QLabel *infoLabel;
    QPushButton *openButton;
    QPushButton *playButton;
    QMediaPlayer *mediaPlayer;
    QVideoWidget *videoWidget;
    QSlider *positionSlider;
};
#endif // MAINWINDOW_H
