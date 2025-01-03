#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建中心部件和布局
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    // 创建打开按钮
    openButton = new QPushButton("打开视频", this);
    buttonLayout->addWidget(openButton);

    // 创建播放/暂停按钮
    playButton = new QPushButton("播放", this);
    playButton->setEnabled(false);
    buttonLayout->addWidget(playButton);

    layout->addLayout(buttonLayout);

    // 创建视频显示部件
    videoWidget = new QVideoWidget(this);
    videoWidget->setMinimumHeight(400);
    layout->addWidget(videoWidget);

    // 创建进度条
    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setEnabled(false);
    layout->addWidget(positionSlider);

    // 创建信息显示标签
    infoLabel = new QLabel(this);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    // 创建媒体播放器
    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);

    // 连接信号和槽
    connect(openButton, &QPushButton::clicked, this, &MainWindow::openVideo);
    connect(playButton, &QPushButton::clicked, this, &MainWindow::playPauseVideo);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::updatePosition);
    connect(positionSlider, &QSlider::sliderMoved, this, &MainWindow::setPosition);

    // 设置窗口大小
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openVideo()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("打开视频文件"),
        "",
        tr("视频文件 (*.mp4 *.avi *.mkv);;所有文件 (*.*)"));

    if (!fileName.isEmpty()) {
        mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
        playButton->setEnabled(true);
        playButton->setText("播放");

        // 显示文件信息
        QFileInfo fileInfo(fileName);
        QString info = QString("文件名: %1\n大小: %2 MB\n路径: %3")
            .arg(fileInfo.fileName())
            .arg(QString::number(fileInfo.size() / 1024.0 / 1024.0, 'f', 2))
            .arg(fileInfo.absolutePath());

        infoLabel->setText(info);
    }
}

void MainWindow::playPauseVideo()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
        playButton->setText("播放");
    } else {
        mediaPlayer->play();
        playButton->setText("暂停");
    }
}

void MainWindow::updateDuration(qint64 duration)
{
    positionSlider->setEnabled(true);
    positionSlider->setRange(0, duration);
}

void MainWindow::updatePosition(qint64 position)
{
    positionSlider->setValue(position);
}

void MainWindow::setPosition(int position)
{
    mediaPlayer->setPosition(position);
}

