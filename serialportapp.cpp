#include "serialportapp.h"
#include "ui_serialportapp.h"

SerialPortApp::SerialPortApp(QWidget *parent) : QWidget(parent), ui(new Ui::SerialPortApp) {
    ui->setupUi(this);

    // 遍历获取当前设备上的所有串口
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    for (int i = 0; i < infos.size(); i++) {
        ui->comCb->addItem(infos.at(i).portName());
    }

    // 设置波特率显示
    QStringList list;
    list << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    ui->rateCb->addItems(list);
    ui->rateCb->setCurrentIndex(7);  // 设置默认波特率为 115200
    list.clear();

    // 设置数据位
    list << "5" << "6" << "7" << "8" << "-1";
    ui->dataCb->addItems(list);
    ui->dataCb->setCurrentIndex(3);  // 设置默认数据位为 8
    list.clear();

    // 设置停止位
    list << "1" << "3" << "2" << "-1";
    ui->stopCb->addItems(list);
    list.clear();

    // 设置校验位
    list << "None" << "NULL" << "Even" << "Odd" << "Space" << "Mark";
    ui->priCb->addItems(list);
    list.clear();

    // 把关闭按钮设置失效
    ui->closeBt->setEnabled(false);

    // 当串口有数据可读时会发送 readyRead 信号
    connect(&mSerial, &QSerialPort::readyRead, this, &SerialPortApp::read_data);

    // 初始化发送、接收的字节数记录
    sendsize = recvsize = 0;
}

SerialPortApp::~SerialPortApp() {
    delete ui;
}

// 打开
void SerialPortApp::on_openBt_clicked() {
    // 配置端口，波特率，数据位，停止位，校验位
    mSerial.setPortName(ui->comCb->currentText());
    mSerial.setBaudRate(ui->rateCb->currentText().toInt());
    mSerial.setDataBits((QSerialPort::DataBits)ui->dataCb->currentText().toInt());
    mSerial.setStopBits((QSerialPort::StopBits)ui->stopCb->currentText().toInt());
    mSerial.setParity((QSerialPort::Parity)ui->priCb->currentText().toInt());

    // 打开设备
    if (mSerial.open(QIODevice::ReadWrite)) {
        ui->closeBt->setEnabled(true);
        ui->openBt->setEnabled(false);
    }
}

// 关闭
void SerialPortApp::on_closeBt_clicked() {
    // 关闭设备
    mSerial.close();
    ui->closeBt->setEnabled(false);
    ui->openBt->setEnabled(true);
}

// 手动发送数据
void SerialPortApp::on_sendBt_clicked() {
    QString data = ui->sendText->toPlainText();
    if (ui->sendHexCb->isChecked()) {
        // 转十六进制：data = 4142 --> 0x41 0x42
        QByteArray array;
        if (data.size() % 2 != 0) {
            data.insert(0, '0');
        }
        for (int i = 0; i < data.size() / 2; i++) {
            QString t = data.mid(2*i, 2);
            bool ok = false;
            int ihex = t.toInt(&ok, 16);
            array.append(ihex);
        }

        int size = mSerial.write(array);  // 发送数据
        sendsize += size;                 // 累计发送的字节数
    } else {
        int size = mSerial.write(data.toUtf8());  // 发送数据
        sendsize += size;
    }
    // 设置显示已发送的字节数
    ui->sendsizelabel->setText(QString::number(sendsize));
}

// 定时自动发送数据
void SerialPortApp::on_autoCheckBox_clicked(bool checked) {
    if (checked) {
        // 获取定时发送周期
        int ms = ui->autotimeEdit->text().toInt();
        if (ms < 100) {
            QMessageBox::warning(this, "time hint", "time should > 100ms");
            ui->autoCheckBox->setChecked(false);

            return;
        }

        // 启动定时器事件
        timerid = this->startTimer(ms);
    } else {
        // 关闭定时器事件
        this->killTimer(timerid);
    }
}

// 定时器事件
void SerialPortApp::timerEvent(QTimerEvent *event) {
    on_sendBt_clicked();
}

// 清空已发送的字节数
void SerialPortApp::on_clearSendSizeBt_clicked() {
    sendsize = 0;
    ui->sendText->clear();
    ui->sendsizelabel->setText("0");
}

// 发送端：十六进制和十进制转换
void SerialPortApp::on_sendHexCb_clicked(bool checked) {
    if (checked) {  // 十进制 --> 十六进制
        QString data = ui->sendText->toPlainText();
        QByteArray array = data.toUtf8().toHex();
        ui->sendText->setText(QString(array));
    } else {  // 十六进制 --> 十进制
        QString data = ui->sendText->toPlainText();
        QByteArray array;
        if (data.size() % 2 != 0) {
            data.insert(0, '0');
        }
        for (int i = 0; i < data.size() / 2; i++) {
            QString t = data.mid(2*i, 2);
            bool ok = false;
            int ihex = t.toInt(&ok, 16);
            array.append(ihex);
        }
        ui->sendText->setText(QString(array));
    }
}

// 接收端：十六进制和十进制转换
void SerialPortApp::on_recvHexCb_clicked(bool checked) {
    if (checked) {  // 十进制 --> 十六进制
        QString data = ui->recvText->toPlainText();
        QByteArray array = data.toUtf8().toHex();
        ui->recvText->setText(QString(array));
    } else {  // 十六进制 --> 十进制
        QString data = ui->recvText->toPlainText();
        QByteArray array;
        if (data.size() % 2 != 0) {
            data.insert(0, '0');
        }
        for (int i = 0; i < data.size() / 2; i++) {
            QString t = data.mid(2*i, 2);
            bool ok = false;
            int ihex = t.toInt(&ok, 16);
            array.append(ihex);
        }
        ui->recvText->setText(QString(array));
    }
}

// 接收串口数据
void SerialPortApp::read_data() {
    // 读到的数据是一个个字节
    QByteArray array = mSerial.readAll();
    recvsize += array.size();  // 显示已接收到的字节数

    if (ui->recvHexCb->isChecked()) {
        ui->recvText->append(array.toHex());
    } else {
        ui->recvText->append(array);
    }

    // 设置显示已接收到的字节数
    ui->recvsizelabel->setText(QString::number(recvsize));
}

// 清空已接收的字节数
void SerialPortApp::on_clearRecvSizeBt_clicked() {
    recvsize = 0;
    ui->recvText->clear();
    ui->recvsizelabel->setText("0");
}

// 选择要发送的文件
void SerialPortApp::on_selectfileBt_clicked() {
    QString path = QFileDialog::getOpenFileName(this);
    ui->filepathEdit->setText(path);
}

// 发送文件
void SerialPortApp::on_sendfileBt_clicked() {
    // 当数据发送完毕后会发出一个信号 &QSerialPort::bytesWritten
    // 每当有效载荷的数据写入到设备当前的写入通道时，就会发出这个信号
    connect(&mSerial, &QSerialPort::bytesWritten, this, &SerialPortApp::send_file_text);

    // 打开文件
    file.setFileName(ui->filepathEdit->text());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    // 获取文件大小
    int filesize = file.size();
    ui->progressBar->setMaximum(filesize);

    // 设置进度条显示
    QByteArray array = file.read(128);  // 每次读取 128 字节内容
    sendfilesize = mSerial.write(array);
    ui->progressBar->setValue(sendfilesize);
}

// 循环（每 128 字节）发送文件
void SerialPortApp::send_file_text(quint64 size) {
    // 设置进度条显示
    QByteArray array = file.read(128);
    quint64 mSize = mSerial.write(array);
    sendfilesize += mSize;
    ui->progressBar->setValue(sendfilesize);

    // 判断文件是否发送完毕
    if (sendfilesize == ui->progressBar->maximum()) {
        file.close();
        disconnect(&mSerial, &QSerialPort::bytesWritten, this, &SerialPortApp::send_file_text);
    }
}
