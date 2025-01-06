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
#include <QRegularExpression>
#include "version.h"  // 添加版本头文件包含

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
    , removeAudioCheckBox(nullptr)
    , processTimeLabel(nullptr)
    , processStartTime()
    , processTimer(nullptr)
    , totalDuration(0)
{
    instance = this;  // 保存实例指针
    ui->setupUi(this);
    
    // 安装消息处理器
    qInstallMessageHandler(messageHandler);
    
    setupUI();
    
    // 创建并显示启动界面
    // hide();  // 先隐藏主窗口
    // SplashWindow *splash = new SplashWindow(size(), pos(), this);
    // connect(splash, &SplashWindow::finished, this, [this]() {
    //     show();  // 启动界面结束后显示主窗口
    // });
    
    // splash->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    setWindowTitle("FilmTool");
    
    // 设置应用程序图标
    setWindowIcon(QIcon(":/images/logo.png"));
    
    // 创建菜单栏
    QMenuBar *menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // 创建帮助菜单
    QMenu *helpMenu = menuBar->addMenu("帮助");
    
    // 创建关于动作
    QAction *aboutAction = new QAction("关于", this);
    helpMenu->addAction(aboutAction);
    
    // 连接关于动作的信号
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
    
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
    // 创建组合框
    QGroupBox *listGroup = new QGroupBox("视频列表", mainSplitter);
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);
    
    // 设置最小宽度
    listGroup->setMinimumWidth(210);
    
    // 创建列表
    videoList = new CustomListWidget(listGroup);
    videoList->setContextMenuPolicy(Qt::CustomContextMenu);
    
    // 设置图标视图模式
    videoList->setViewMode(QListWidget::IconMode);
    videoList->setIconSize(QSize(160, 90));
    videoList->setSpacing(10);
    videoList->setMovement(QListWidget::Snap);
    videoList->setResizeMode(QListWidget::Adjust);
    videoList->setWrapping(false);
    videoList->setUniformItemSizes(true);
    videoList->setFlow(QListWidget::TopToBottom);
    videoList->setGridSize(QSize(180, 130));
    
    // 启用拖放
    videoList->setDragEnabled(true);
    videoList->setAcceptDrops(true);
    videoList->setDropIndicatorShown(true);
    videoList->setDefaultDropAction(Qt::MoveAction);
    videoList->setDragDropMode(QListWidget::InternalMove);
    
    // 将列表添加到布局中
    listLayout->addWidget(videoList);
    
    // 将组合框添加到分割器中
    mainSplitter->addWidget(listGroup);

    // 创建右键菜单
    listContextMenu = new QMenu(this);
    listContextMenu->addAction("添加视频", this, &MainWindow::addVideoToList);
    listContextMenu->addAction("删除视频", this, &MainWindow::removeVideoFromList);

    // 连接信号
    connect(videoList, &QListWidget::customContextMenuRequested,
            this, &MainWindow::showListContextMenu);
    connect(videoList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onVideoItemDoubleClicked);
    
    // 监听模型变化，用于重新排列项目
    connect(videoList->model(), &QAbstractItemModel::rowsMoved,
            this, &MainWindow::onItemMoved);
    connect(videoList->model(), &QAbstractItemModel::rowsRemoved,
            this, &MainWindow::onItemMoved);
}

void MainWindow::createVideoPlayer()
{
    videoWidget = new QVideoWidget(this);
    videoWidget->setMinimumHeight(400);

    mediaPlayer = new QMediaPlayer(this);
    
    // 创建音频输出
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);
    
    mediaPlayer->setVideoOutput(videoWidget);
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
    // 创建配置区域的组合框
    QGroupBox *settingsGroup = new QGroupBox("配置选项", this);
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsGroup);
    
    // 创建消除音轨复选框
    removeAudioCheckBox = new QCheckBox("消除音轨", this);
    removeAudioCheckBox->setToolTip("勾选此项将在合并时移除所有视频的音轨");
    settingsLayout->addWidget(removeAudioCheckBox);
    
    // 创建清晰度选择
    QHBoxLayout *qualityLayout = new QHBoxLayout();
    QLabel *qualityLabel = new QLabel("清晰度:", this);
    qualityComboBox = new QComboBox(this);
    qualityComboBox->addItem("原画质量", "original");
    qualityComboBox->addItem("中等画质", "medium");
    qualityComboBox->addItem("低清画质", "low");
    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(qualityComboBox);
    settingsLayout->addLayout(qualityLayout);
    
    // 创建帧率选择
    QHBoxLayout *fpsLayout = new QHBoxLayout();
    QLabel *fpsLabel = new QLabel("帧率:", this);
    fpsComboBox = new QComboBox(this);
    fpsComboBox->addItem("60 FPS", 60);
    fpsComboBox->addItem("30 FPS", 30);
    fpsLayout->addWidget(fpsLabel);
    fpsLayout->addWidget(fpsComboBox);
    settingsLayout->addLayout(fpsLayout);
    
    // 在配置区域添加处理时间显示
    QHBoxLayout *timeLayout = new QHBoxLayout();
    QLabel *timeLabel = new QLabel("处理时间:", this);
    processTimeLabel = new QLabel("00:00:00", this);
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(processTimeLabel);
    timeLayout->addStretch();
    settingsLayout->addLayout(timeLayout);
    
    // 添加一些空白空间
    settingsLayout->addStretch();
    
    // 创建日志输出组合框
    configGroup = new QGroupBox("日志输出", this);
    QVBoxLayout *logLayout = new QVBoxLayout(configGroup);
    
    // 创建日志输出框
    logOutput = new QTextEdit(this);
    logOutput->setReadOnly(true);
    logOutput->setFont(QFont("Consolas", 9));
    logOutput->setLineWrapMode(QTextEdit::NoWrap);
    logLayout->addWidget(logOutput);
    
    // 将两个组合框添加到右侧布局
    QVBoxLayout *rightLayout = qobject_cast<QVBoxLayout*>(rightWidget->layout());
    rightLayout->addWidget(settingsGroup);
    rightLayout->addWidget(configGroup);
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
            item->setSizeHint(QSize(180, 130));  // 设置项目大小，包含文本空间
            
            // 设置默认图标
            item->setIcon(QIcon::fromTheme("video-x-generic"));
            
            videoList->addItem(item);
            
            // 异步提取缩略图
            extractThumbnail(fileName, item);
        }
        mergeButton->setEnabled(videoList->count() > 0);  // 只要有视频就使能
        mergeButton->setText("合并导出");  // 更新按钮文字
    }
}

void MainWindow::removeVideoFromList()
{
    QListWidgetItem *currentItem = videoList->currentItem();
    if (currentItem) {
        delete currentItem;
        mergeButton->setEnabled(videoList->count() > 0);  // 更新按钮状态
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
    // 重置并开始计时
    processStartTime = QTime::currentTime();
    if (!processTimer) {
        processTimer = new QTimer(this);
        connect(processTimer, &QTimer::timeout, this, [this]() {
            QTime currentTime = QTime::currentTime();
            int elapsed = processStartTime.msecsTo(currentTime);
            QTime displayTime = QTime(0, 0).addMSecs(elapsed);
            processTimeLabel->setText(displayTime.toString("hh:mm:ss"));
        });
    }
    processTimer->start(1000);  // 每秒更新一次
    
    // 计算所有视频的总时长
    totalDuration = 0;
    for (int i = 0; i < videoList->count(); ++i) {
        QProcess probe;
        QStringList arguments;
        QString videoPath = videoList->item(i)->data(Qt::UserRole).toString();
        
        arguments << "-v" << "error"
                 << "-show_entries" << "format=duration"
                 << "-of" << "default=noprint_wrappers=1:nokey=1"
                 << videoPath;
        
        QString ffprobePath = QCoreApplication::applicationDirPath() + "/ffprobe.exe";
        probe.start(ffprobePath, arguments);
        probe.waitForFinished();
        
        QString output = probe.readAllStandardOutput();
        double duration = output.toDouble();
        totalDuration += qint64(duration * 1000);  // 转换为毫秒
    }
    
    // 创建进度对话框
    progressDialog = new QProgressDialog("正在处理视频...", "取消", 0, 100, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setAutoClose(true);
    progressDialog->setAutoReset(true);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);
    
    // 清空之前的日志
    logOutput->clear();
    appendLog("开始处理视频...");
    
    // 构建 FFmpeg 命令
    QStringList arguments;
    arguments << "-y";  // 覆盖输出文件
    
    // 添加多线程支持
    arguments << "-threads" << "0";  // 0表示自动选择最优线程数
    
    if (videoList->count() > 1) {
        // 多个视频需要合并
        QString mergeList = createMergeList();
        arguments << "-f" << "concat" << "-safe" << "0" << "-i" << mergeList;
    } else {
        // 单个视频直接处理
        QString inputFile = videoList->item(0)->data(Qt::UserRole).toString();
        arguments << "-i" << inputFile;
    }
    
    // 根据清晰度设置视频参数
    QString quality = qualityComboBox->currentData().toString();
    if (quality == "medium") {
        arguments << "-vf" << "scale=1280:720";  // 720p
        arguments << "-b:v" << "2M";  // 2Mbps比特率
    } else if (quality == "low") {
        arguments << "-vf" << "scale=854:480";   // 480p
        arguments << "-b:v" << "1M";  // 1Mbps比特率
    }
    
    // 设置帧率
    int fps = fpsComboBox->currentData().toInt();
    arguments << "-r" << QString::number(fps);
    
    // 根据复选框状态决定是否添加音频
    if (removeAudioCheckBox->isChecked()) {
        arguments << "-an";  // 移除音频
    }
    
    // 如果不是原画质量，使用更高压缩率的编码器
    if (quality != "original") {
        arguments << "-c:v" << "libx264" << "-preset" << "medium";
        // 添加 x264 的多线程优化参数
        arguments << "-x264-params" << "threads=auto";
    } else {
        arguments << "-c:v" << "copy";  // 原画质量时直接复制视频流
    }
    
    if (!removeAudioCheckBox->isChecked() && quality != "original") {
        arguments << "-c:a" << "aac" << "-b:a" << "128k";  // 音频编码设置
    }
    
    arguments << outputFile;
    
    // 连接取消信号
    connect(progressDialog, &QProgressDialog::canceled, this, [this]() {
        if (mergeProcess && mergeProcess->state() == QProcess::Running) {
            mergeProcess->kill();  // 强制终止进程
            appendLog("用户取消了处理操作");
            
            // 清理临时文件
            if (videoList->count() > 1) {
                QString mergeListPath = QDir::tempPath() + "/merge_list.txt";
                QFile::remove(mergeListPath);
            }
            
            // 如果输出文件已经创建，则删除它
            if (QFile::exists(currentMergeOutput)) {
                QFile::remove(currentMergeOutput);
            }
        }
    });
    
    // 启动 FFmpeg 进程
    QString ffmpegPath = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    qDebug() << "FFmpeg 命令:" << ffmpegPath << arguments.join(' ');
    
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
                
                // 解析FFmpeg输出的时间信息
                static QRegularExpression timeRegex("time=([0-9:.]+)");
                QRegularExpressionMatch match = timeRegex.match(error);
                if (match.hasMatch()) {
                    QString timeStr = match.captured(1);
                    QStringList timeParts = timeStr.split(":");
                    if (timeParts.size() == 3) {
                        qint64 hours = timeParts[0].toInt();
                        qint64 minutes = timeParts[1].toInt();
                        double seconds = timeParts[2].toDouble();
                        qint64 currentMs = (hours * 3600 + minutes * 60 + seconds) * 1000;
                        
                        // 更新进度条
                        if (totalDuration > 0) {
                            int progress = (currentMs * 100) / totalDuration;
                            progressDialog->setValue(progress);
                        }
                    }
                }
            });

    mergeProcess->start(ffmpegPath, arguments);
}

void MainWindow::onMergeProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // 停止计时器
    if (processTimer) {
        processTimer->stop();
    }
    
    qDebug() << "处理进程结束 - 退出码:" << exitCode << "退出状态:" << exitStatus;
    
    progressDialog->close();
    delete progressDialog;
    progressDialog = nullptr;
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        QMessageBox::information(this, "完成", "视频处理完成！");
        
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
        // 如果不是用户取消的（exitCode != -2），则显示错误消息
        if (exitCode != -2) {
            QString errorMessage = "视频处理失败！\n";
            errorMessage += "退出码: " + QString::number(exitCode) + "\n";
            errorMessage += "错误输出: " + mergeProcess->readAllStandardError();
            QMessageBox::critical(this, "错误", errorMessage);
        }
    }

    mergeProcess->deleteLater();
    mergeProcess = nullptr;
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

QString MainWindow::createTempThumbnailPath()
{
    // 创建临时文件路径
    QString tempPath = QDir::tempPath() + "/FilmTool_thumb_" + 
                      QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";
    return tempPath;
}

void MainWindow::extractThumbnail(const QString &videoPath, QListWidgetItem *item)
{
    QString thumbnailPath = createTempThumbnailPath();
    
    // 创建 FFmpeg 进程
    QProcess *process = new QProcess(this);
    
    // 在进程完成时处理缩略图
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process, thumbnailPath, item](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitCode == 0) {
                    QPixmap thumbnail(thumbnailPath);
                    if (!thumbnail.isNull()) {
                        // 设置缩略图为图标
                        item->setIcon(QIcon(thumbnail));
                    }
                }
                // 清理临时文件
                QFile::remove(thumbnailPath);
                process->deleteLater();
            });

    // 构建 FFmpeg 命令
    QStringList arguments;
    arguments << "-y"
              << "-i" << videoPath
              << "-ss" << "00:00:01"  // 从1秒处截图
              << "-vframes" << "1"
              << "-s" << "160x90"     // 缩略图尺寸
              << thumbnailPath;

    // 启动进程
    QString ffmpegPath = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    process->start(ffmpegPath, arguments);
}

void MainWindow::onItemMoved()
{
    // 收集所有项目
    QList<QListWidgetItem*> items;
    while (videoList->count() > 0) {
        items.append(videoList->takeItem(0));
    }

    // 重新按顺序添加项目
    for (QListWidgetItem* item : items) {
        videoList->addItem(item);
    }
}

void MainWindow::showAboutDialog()
{
    QString aboutText = QString("FilmTool 视频处理工具\n\n版本号: %1\n\n"
                              "一个简单的视频处理工具，支持视频合并、转码等功能。")
                              .arg(APP_VERSION);
    
    QMessageBox::about(this, "关于 FilmTool", aboutText);
}
