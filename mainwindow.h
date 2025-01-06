#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <QSplitter>
#include <QMenu>
#include <QGroupBox>
#include <QProgressDialog>
#include <QProcess>
#include <QTextEdit>
#include <QLabel>
#include "customlistwidget.h"
#include <QAudioOutput>
#include <QCheckBox>
#include <QComboBox>
#include <QTime>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void playPauseVideo();
    void updateDuration(qint64 duration);
    void updatePosition(qint64 position);
    void setPosition(int position);
    void showListContextMenu(const QPoint &pos);
    void addVideoToList();
    void removeVideoFromList();
    void onVideoItemDoubleClicked(QListWidgetItem *item);
    void mergeVideos();
    void onMergeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void appendLog(const QString &text);
    void onItemMoved();
    void showAboutDialog();

private:
    void setupUI();
    void createVideoList();
    void createVideoPlayer();
    void createConfigArea();
    QWidget* createControlButtons();
    QString createMergeList();
    void startMerging(const QString &outputFile);
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static MainWindow *instance;
    
    QString createTempThumbnailPath();
    void extractThumbnail(const QString &videoPath, QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QSplitter *mainSplitter;
    
    // 左侧视频列表
    CustomListWidget *videoList;
    QMenu *listContextMenu;

    // 右侧视频播放区域
    QWidget *rightWidget;
    QVideoWidget *videoWidget;
    QMediaPlayer *mediaPlayer;
    QSlider *positionSlider;
    QPushButton *playButton;

    // 右侧配置区域
    QGroupBox *configGroup;

    // 合并相关
    QPushButton *mergeButton;
    QProcess *mergeProcess;
    QProgressDialog *progressDialog;
    QString currentMergeOutput;

    QTextEdit *logOutput;

    QLabel *currentTimeLabel;
    QLabel *totalTimeLabel;

    QProcess *thumbnailProcess;  // 用于提取缩略图

    QAudioOutput* audioOutput;

    QCheckBox *removeAudioCheckBox;  // 添加消除音轨复选框

    QComboBox *qualityComboBox;    // 清晰度下拉框
    QComboBox *fpsComboBox;        // 帧率下拉框

    QLabel *processTimeLabel;    // 处理时间显示标签
    QTime processStartTime;      // 处理开始时间
    QTimer *processTimer;        // 处理时间计时器
    qint64 totalDuration;        // 所有视频的总时长
};
#endif // MAINWINDOW_H
