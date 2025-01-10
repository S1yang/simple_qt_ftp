#include "ftpuploader.h"
#include <QUrl>
#include <QDebug>
#include <QNetworkProxy>


FtpUploader::FtpUploader(QObject *parent)
    : QObject(parent)
{
    // 如果需要可在此处进行 QNetworkAccessManager 的更多设置
    // 绕过系统代理
    QNetworkProxy noProxy(QNetworkProxy::NoProxy);
    QNetworkProxy::setApplicationProxy(noProxy);
}

void FtpUploader::uploadToFtp(const QString &filePath,
                              const QString &ftpAddress)
{


    // QTcpSocket socket;

    // QNetworkProxy proxy;
    // proxy.setType(QNetworkProxy::NoProxy);
    // socket.setProxy(proxy);

    // socket.connectToHost("192.168.144.152", 21);

    // if (!socket.waitForConnected(3000)) {
    //     qDebug() << "无法连接FTP服务器, error:" << socket.errorString();
    // } else {
    //     qDebug() << "已连接到FTP服务器端口21, 可以尝试后续FTP命令。";
    //     // 可以socket.write("USER cat\r\n") 等，但这就要手动实现FTP交互了
    // }

    int timeoutMs = 10000;
    // 确保上一次的资源已经释放
    if (m_file) {
        if (m_file->isOpen()) {
            m_file->close();
        }
        delete m_file;
        m_file = nullptr;
    }

    if (m_reply) {
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }

    // 打开本地文件
    m_file = new QFile(filePath, this);
    if (!m_file->open(QIODevice::ReadOnly)) {
        QString errorMsg = QString("无法打开文件: %1").arg(filePath);
        qWarning() << errorMsg;
        emit uploadError(errorMsg);
        delete m_file;
        m_file = nullptr;
        return;
    }

    // 配置目标 URL，例如 ftp://192.168.144.152/home/cat/imgs/xxx.png
    QUrl url(ftpAddress);
    // 设置 FTP 登录用户名和密码
    url.setUserName("cat");
    url.setPassword("yj666");

    // 如果使用的不是默认21端口，可以设置自定义端口
    // url.setPort(21);

    // 生成网络请求
    QNetworkRequest request(url);


    // 发起 PUT 请求，将本地文件内容上传
    m_reply = m_manager.put(request, m_file);

    // 绑定上传进度
    connect(m_reply, &QNetworkReply::uploadProgress,
            this, &FtpUploader::onUploadProgress);

    // 上传完成（成功或失败）时触发
    connect(m_reply, &QNetworkReply::finished,
            this, &FtpUploader::onFinished);

    // 网络出错时触发
    connect(m_reply,
            QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this,
            &FtpUploader::onErrorOccurred);

    // 5) 设置超时定时器
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &FtpUploader::onTimeout);
    m_timer->start(timeoutMs);

    qDebug() << "开始上传文件到:" << ftpAddress
             << ", 超时时长:" << timeoutMs << "ms";
}

void FtpUploader::onUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    emit uploadProgress(bytesSent, bytesTotal);
}

void FtpUploader::onFinished()
{
    // 如果没有错误，会走到这里
    // 停止定时器
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }

    if (m_reply) {
        // 检查是否真的没有错误
        if (m_reply->error() == QNetworkReply::NoError) {
            qDebug() << "上传成功";
            // QMessageBox::information(nullptr, "上传成功", "文件已成功上传到服务器！");
            emit uploadFinished();
        }
        // 释放资源
        m_reply->deleteLater();
        m_reply = nullptr;

    }

    if (m_file) {
        if (m_file->isOpen()) {
            m_file->close();
        }
        delete m_file;
        m_file = nullptr;
    }
}

void FtpUploader::onErrorOccurred(QNetworkReply::NetworkError code)
{
    // 停止定时器
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    // 这里会处理各种网络错误，包含超时、无法连接FTP服务器等
    if (m_reply) {
        QString errString = QString("FTP 上传出错 (code:%1): %2")
                                .arg(code)
                                .arg(m_reply->errorString());
        qWarning() << errString;

        // 释放 reply
        m_reply->deleteLater();
        m_reply = nullptr;

        // 关闭文件
        if (m_file) {
            if (m_file->isOpen()) {
                m_file->close();
            }
            delete m_file;
            m_file = nullptr;
        }

        // 通知 QML 或外部逻辑
        emit uploadError(errString);
    }
}

void FtpUploader::onTimeout()
{
    qWarning() << "上传超时，主动中止...";
    QMessageBox::information(nullptr, "上传超时", "请检查网络是否正确连接");
    if (m_reply) {
        m_reply->abort();  // 触发 onErrorOccurred -> OperationCanceledError
    }
}
