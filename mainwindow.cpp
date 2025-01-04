#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QTimer>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>
#include <QTextEdit>
#include <QScrollBar>
#include <QFont>

MainWindow* MainWindow::instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mergeProcess(nullptr)
    , progressDialog(nullptr)
    , mediaPlayer(nullptr)
    , positionSlider(nullptr)
    , currentTimeLabel(nullptr)
    , totalTimeLabel(nullptr)
{
    instance = this;  // 保存实例指针
    ui->setupUi(this);
    
    // 安装消息处理器
    qInstallMessageHandler(messageHandler);
    
    setupUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);

    // 创建左侧视频列表
    createVideoList();

    // 创建右侧区域
    rightWidget = new QWidget(mainSplitter);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    
    // 创建视频播放区域
    createVideoPlayer();
    rightLayout->addWidget(videoWidget);
    rightLayout->addWidget(createControlButtons());

    // 创建配置区域
    createConfigArea();
    rightLayout->addWidget(configGroup);

    // 设置分割器比例
    mainSplitter->setStretchFactor(0, 1);  // 左侧列表
    mainSplitter->setStretchFactor(1, 3);  // 右侧区域

    // 设置窗口大小
    resize(1200, 800);
}

void MainWindow::createVideoList()
{
    videoList = new QListWidget(mainSplitter);
    videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // 创建右键菜单
    listContextMenu = new QMenu(this);
    listContextMenu->addAction("添加视频", this, &MainWindow::addVideoToList);
    listContextMenu->addAction("删除视频", this, &MainWindow::removeVideoFromList);

    // 连接信号
    connect(videoList, &QListWidget::customContextMenuRequested,
            this, &MainWindow::showListContextMenu);
    connect(videoList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onVideoItemDoubleClicked);
}

void MainWindow::createVideoPlayer()
{
    videoWidget = new QVideoWidget(this);
    videoWidget->setMinimumHeight(400);

    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);

    // 注意：这里移除 positionSlider 的信号连接，因为它还没有被创建
    // 我们会在 createControlButtons 之后再连接信号
}

QWidget* MainWindow::createControlButtons()
{
    QWidget *controlWidget = new QWidget(this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setContentsMargins(5, 0, 5, 0);

    // 合并按钮
    mergeButton = new QPushButton("合并视频", this);
    mergeButton->setEnabled(false);
    mergeButton->setFixedWidth(80);
    controlLayout->addWidget(mergeButton);

    // 播放/暂停按钮
    playButton = new QPushButton(this);
    playButton->setEnabled(false);
    playButton->setFixedWidth(40);  // 较小的宽度
    playButton->setIcon(QIcon::fromTheme("media-playback-start"));  // 使用播放图标
    controlLayout->addWidget(playButton);

    controlLayout->addSpacing(10);

    // 当前时间标签
    currentTimeLabel = new QLabel("00:00:00", this);
    currentTimeLabel->setFixedWidth(60);
    currentTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    controlLayout->addWidget(currentTimeLabel);
    
    // 进度条
    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setEnabled(false);
    controlLayout->addWidget(positionSlider);
    
    // 总时间标签
    totalTimeLabel = new QLabel("00:00:00", this);
    totalTimeLabel->setFixedWidth(60);
    totalTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    controlLayout->addWidget(totalTimeLabel);
    
    // 连接信号
    connect(playButton, &QPushButton::clicked, this, &MainWindow::playPauseVideo);
    connect(mergeButton, &QPushButton::clicked, this, &MainWindow::mergeVideos);
    
    // 在这里连接 positionSlider 的信号
    connect(positionSlider, &QSlider::sliderMoved, this, &MainWindow::setPosition);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::updatePosition);

    return controlWidget;
}

void MainWindow::createConfigArea()
{
    configGroup = new QGroupBox("日志输出", this);
    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);
    
    // 创建日志输出框
    logOutput = new QTextEdit(this);
    logOutput->setReadOnly(true);
    logOutput->setFont(QFont("Consolas", 9));  // 使用等宽字体
    logOutput->setLineWrapMode(QTextEdit::NoWrap);  // 不自动换行
    configLayout->addWidget(logOutput);
}

void MainWindow::showListContextMenu(const QPoint &pos)
{
    listContextMenu->exec(videoList->mapToGlobal(pos));
}

void MainWindow::addVideoToList()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("打开视频文件"), "",
        tr("视频文件 (*.mp4 *.avi *.mkv);;所有文件 (*.*)"));

    if (!fileNames.isEmpty()) {
        for (const QString &fileName : fileNames) {
            QListWidgetItem *item = new QListWidgetItem(QFileInfo(fileName).fileName());
            item->setData(Qt::UserRole, fileName);  // 存储完整路径
            videoList->addItem(item);
        }
        mergeButton->setEnabled(videoList->count() > 1);  // 当列表中有多个视频时启用合并按钮
    }
}

void MainWindow::removeVideoFromList()
{
    QListWidgetItem *currentItem = videoList->currentItem();
    if (currentItem) {
        delete currentItem;
        mergeButton->setEnabled(videoList->count() > 1);
    }
}

void MainWindow::onVideoItemDoubleClicked(QListWidgetItem *item)
{
    QString fileName = item->data(Qt::UserRole).toString();
    mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
    playButton->setEnabled(true);
    playButton->setIcon(QIcon::fromTheme("media-playback-start"));  // 设置播放图标
    
    // 重置时间标签
    currentTimeLabel->setText("00:00:00");
    totalTimeLabel->setText("00:00:00");
}

// 保持原有的播放控制相关函数
void MainWindow::playPauseVideo()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        playButton->setIcon(QIcon::fromTheme("media-playback-start"));  // 切换为播放图标
    } else {
        mediaPlayer->play();
        playButton->setIcon(QIcon::fromTheme("media-playback-pause"));  // 切换为暂停图标
    }
}

void MainWindow::updateDuration(qint64 duration)
{
    qDebug() << "updateDuration" << duration;
    positionSlider->setEnabled(true);
    positionSlider->setRange(0, duration);
    
    // 更新总时间标签
    qint64 seconds = duration / 1000;
    qint64 hours = seconds / 3600;
    qint64 minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;
    totalTimeLabel->setText(QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::updatePosition(qint64 position)
{
    if (!positionSlider->isSliderDown())
        positionSlider->setValue(position);

    // 更新当前时间标签
    qint64 seconds = position / 1000;
    qint64 hours = seconds / 3600;
    qint64 minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;
    currentTimeLabel->setText(QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::setPosition(int position)
{
    mediaPlayer->setPosition(position);
}

QString MainWindow::createMergeList()
{
    QString tempPath = QDir::tempPath() + "/merge_list.txt";
    QFile file(tempPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        for (int i = 0; i < videoList->count(); ++i) {
            QListWidgetItem *item = videoList->item(i);
            QString filePath = item->data(Qt::UserRole).toString();
            stream << "file '" << filePath.replace("'", "\\'") << "'\n";
        }
        file.close();
    }
    return tempPath;
}

void MainWindow::mergeVideos()
{
    if (videoList->count() < 2) return;

    // 获取输出文件名
    QString outputFile = QFileDialog::getSaveFileName(this,
        tr("保存合并后的视频"), "",
        tr("MP4 视频 (*.mp4)"));

    if (outputFile.isEmpty()) return;

    currentMergeOutput = outputFile;
    startMerging(outputFile);
}

void MainWindow::startMerging(const QString &outputFile)
{
    // 清空之前的日志
    logOutput->clear();
    appendLog("开始合并视频...");
    
    QString mergeList = createMergeList();
    qDebug() << "合并列表文件路径:" << mergeList;
    
    // 创建进度对话框，设置为不确定模式
    progressDialog = new QProgressDialog("正在合并视频...", "取消", 0, 0, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setAutoClose(true);
    progressDialog->setAutoReset(true);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);
    
    // 创建并配置合并进程
    mergeProcess = new QProcess(this);
    connect(mergeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onMergeProcessFinished);
    
    // 添加错误输出处理
    connect(mergeProcess, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
                qDebug() << "进程错误:" << error;
            });
    
    // 添加标准输出和错误输出的处理
    connect(mergeProcess, &QProcess::readyReadStandardOutput,
            this, [this]() {
                QString output = QString::fromLocal8Bit(mergeProcess->readAllStandardOutput());
                appendLog(output);
            });
    
    connect(mergeProcess, &QProcess::readyReadStandardError,
            this, [this]() {
                QString error = QString::fromLocal8Bit(mergeProcess->readAllStandardError());
                appendLog(error);
            });

    // 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString ffmpegPath = appDir + "/ffmpeg.exe";
    qDebug() << "FFmpeg 路径:" << ffmpegPath;

    // 构建 FFmpeg 命令
    QStringList arguments;
    arguments << "-y"  // 添加 -y 参数，自动覆盖已存在的文件
              << "-f" << "concat"
              << "-safe" << "0"
              << "-i" << mergeList
              << "-c" << "copy"
              << outputFile;
    
    qDebug() << "FFmpeg 命令:" << ffmpegPath << arguments.join(' ');
    mergeProcess->start(ffmpegPath, arguments);
}

void MainWindow::onMergeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "合并进程结束 - 退出码:" << exitCode << "退出状态:" << exitStatus;
    
    progressDialog->close();
    delete progressDialog;
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        QMessageBox::information(this, "完成", "视频合并完成！");
        
        // 断开所有之前的连接
        disconnect(mediaPlayer, &QMediaPlayer::mediaStatusChanged, nullptr, nullptr);
        disconnect(mediaPlayer, &QMediaPlayer::errorOccurred, nullptr, nullptr);
        disconnect(mediaPlayer, &QMediaPlayer::durationChanged, nullptr, nullptr);
        
        // 重新连接信号
        connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged,
                this, [this](QMediaPlayer::MediaStatus status) {
                    qDebug() << "媒体状态变化:" << status;
                    switch(status) {
                        case QMediaPlayer::NoMedia:
                            qDebug() << "状态: 无媒体";
                            break;
                        case QMediaPlayer::LoadingMedia:
                            qDebug() << "状态: 加载中";
                            break;
                        case QMediaPlayer::LoadedMedia:
                            qDebug() << "状态: 加载完成，时长:" << mediaPlayer->duration();
                            playButton->setEnabled(true);
                            playButton->setIcon(QIcon::fromTheme("media-playback-start"));
                            break;
                        case QMediaPlayer::InvalidMedia:
                            qDebug() << "状态: 无效媒体";
                            break;
                        default:
                            qDebug() << "状态: 其他状态" << status;
                    }
                });

        // 连接错误信号
        connect(mediaPlayer, &QMediaPlayer::errorOccurred,
                this, [this](QMediaPlayer::Error error, const QString &errorString) {
                    qDebug() << "媒体播放器错误:" << error << errorString;
                });

        // 连接时长变化信号
        connect(mediaPlayer, &QMediaPlayer::durationChanged,
                this, [this](qint64 duration) {
                    qDebug() << "时长直接变化信号:" << duration;
                    updateDuration(duration);  // 在这里更新时长
                });

        qDebug() << "开始加载新视频:" << currentMergeOutput;
        
        // 尝试清除媒体播放器的状态
        mediaPlayer->stop();
        mediaPlayer->setSource(QUrl());  // 清除当前源
        
        // 等待确保NoMedia状态
        if (mediaPlayer->mediaStatus() != QMediaPlayer::NoMedia) {
            qDebug() << "等待媒体播放器清除状态...";
            mediaPlayer->stop();
            mediaPlayer->setSource(QUrl());
        }
        
        // 设置新的源
        mediaPlayer->setSource(QUrl::fromLocalFile(currentMergeOutput));
        
        // 强制重新加载
        mediaPlayer->stop();
        mediaPlayer->play();
        mediaPlayer->pause();
    } else {
        QString errorMessage = "视频合并失败！\n";
        errorMessage += "退出码: " + QString::number(exitCode) + "\n";
        errorMessage += "错误输出: " + mergeProcess->readAllStandardError();
        QMessageBox::critical(this, "错误", errorMessage);
    }

    mergeProcess->deleteLater();
}

void MainWindow::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (instance) {
        QString txt;
        switch (type) {
            case QtDebugMsg:
                txt = QString("Debug: %1").arg(msg);
                break;
            case QtWarningMsg:
                txt = QString("Warning: %1").arg(msg);
                break;
            case QtCriticalMsg:
                txt = QString("Critical: %1").arg(msg);
                break;
            case QtFatalMsg:
                txt = QString("Fatal: %1").arg(msg);
                break;
            default:
                txt = msg;
                break;
        }
        instance->appendLog(txt);
    }
}

void MainWindow::appendLog(const QString &text)
{
    logOutput->append(text.trimmed());
    // 滚动到底部
    QScrollBar *scrollBar = logOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
