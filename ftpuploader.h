#ifndef FTPUPLOADER_H
#define FTPUPLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>
#include <QMessageBox>



class FtpUploader : public QObject
{
    Q_OBJECT
public:
    explicit FtpUploader(QObject *parent = nullptr);

    Q_INVOKABLE void uploadToFtp(const QString &filePath,
                                 const QString &ftpAddress);

signals:
    /// 上传过程中的信号
    void uploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void uploadFinished();
    void uploadError(const QString &errorString);

private slots:
    void onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void onFinished();
    void onErrorOccurred(QNetworkReply::NetworkError code);
    void onTimeout();

private:
    QNetworkAccessManager m_manager;
    QNetworkReply *m_reply = nullptr;
    QFile *m_file = nullptr;
    QTimer *m_timer = nullptr;
};

#endif // FTPUPLOADER_H
