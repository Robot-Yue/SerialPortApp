#ifndef SERIALPORTAPP_H
#define SERIALPORTAPP_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QList>
#include <QStringList>
#include <QString>
#include <QIODevice>
#include <QDebug>
#include <QByteArray>
#include <QMessageBox>
#include <QTimerEvent>
#include <QFileDialog>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class SerialPortApp; }
QT_END_NAMESPACE

class SerialPortApp : public QWidget {
    Q_OBJECT

public:
    SerialPortApp(QWidget *parent = nullptr);
    ~SerialPortApp();

    void timerEvent(QTimerEvent *event);

private slots:
    void on_openBt_clicked();
    void on_closeBt_clicked();
    void on_sendBt_clicked();
    void on_autoCheckBox_clicked(bool checked);
    void on_clearSendSizeBt_clicked();
    void on_sendHexCb_clicked(bool checked);
    void on_recvHexCb_clicked(bool checked);

    void read_data();

    void on_clearRecvSizeBt_clicked();
    void on_selectfileBt_clicked();
    void on_sendfileBt_clicked();

    void send_file_text(quint64 size);

private:
    Ui::SerialPortApp *ui;
    QSerialPort mSerial;
    int timerid;
    qint32 sendsize;
    qint32 recvsize;
    QFile file;  // ·¢ËÍÎÄ¼þ
    qint32 sendfilesize;
};
#endif // SERIALPORTAPP_H
