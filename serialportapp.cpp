#include "serialportapp.h"
#include "ui_serialportapp.h"

SerialPortApp::SerialPortApp(QWidget *parent) : QWidget(parent), ui(new Ui::SerialPortApp) {
    ui->setupUi(this);

    // ������ȡ��ǰ�豸�ϵ����д���
    QList<QSerialPortInfo> infos = QSerialPortInfo::availablePorts();
    for (int i = 0; i < infos.size(); i++) {
        ui->comCb->addItem(infos.at(i).portName());
    }

    // ���ò�������ʾ
    QStringList list;
    list << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    ui->rateCb->addItems(list);
    ui->rateCb->setCurrentIndex(7);  // ����Ĭ�ϲ�����Ϊ 115200
    list.clear();

    // ��������λ
    list << "5" << "6" << "7" << "8" << "-1";
    ui->dataCb->addItems(list);
    ui->dataCb->setCurrentIndex(3);  // ����Ĭ������λΪ 8
    list.clear();

    // ����ֹͣλ
    list << "1" << "3" << "2" << "-1";
    ui->stopCb->addItems(list);
    list.clear();

    // ����У��λ
    list << "None" << "NULL" << "Even" << "Odd" << "Space" << "Mark";
    ui->priCb->addItems(list);
    list.clear();

    // �ѹرհ�ť����ʧЧ
    ui->closeBt->setEnabled(false);

    // �����������ݿɶ�ʱ�ᷢ�� readyRead �ź�
    connect(&mSerial, &QSerialPort::readyRead, this, &SerialPortApp::read_data);

    // ��ʼ�����͡����յ��ֽ�����¼
    sendsize = recvsize = 0;
}

SerialPortApp::~SerialPortApp() {
    delete ui;
}

// ��
void SerialPortApp::on_openBt_clicked() {
    // ���ö˿ڣ������ʣ�����λ��ֹͣλ��У��λ
    mSerial.setPortName(ui->comCb->currentText());
    mSerial.setBaudRate(ui->rateCb->currentText().toInt());
    mSerial.setDataBits((QSerialPort::DataBits)ui->dataCb->currentText().toInt());
    mSerial.setStopBits((QSerialPort::StopBits)ui->stopCb->currentText().toInt());
    mSerial.setParity((QSerialPort::Parity)ui->priCb->currentText().toInt());

    // ���豸
    if (mSerial.open(QIODevice::ReadWrite)) {
        ui->closeBt->setEnabled(true);
        ui->openBt->setEnabled(false);
    }
}

// �ر�
void SerialPortApp::on_closeBt_clicked() {
    // �ر��豸
    mSerial.close();
    ui->closeBt->setEnabled(false);
    ui->openBt->setEnabled(true);
}

// �ֶ���������
void SerialPortApp::on_sendBt_clicked() {
    QString data = ui->sendText->toPlainText();
    if (ui->sendHexCb->isChecked()) {
        // תʮ�����ƣ�data = 4142 --> 0x41 0x42
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

        int size = mSerial.write(array);  // ��������
        sendsize += size;                 // �ۼƷ��͵��ֽ���
    } else {
        int size = mSerial.write(data.toUtf8());  // ��������
        sendsize += size;
    }
    // ������ʾ�ѷ��͵��ֽ���
    ui->sendsizelabel->setText(QString::number(sendsize));
}

// ��ʱ�Զ���������
void SerialPortApp::on_autoCheckBox_clicked(bool checked) {
    if (checked) {
        // ��ȡ��ʱ��������
        int ms = ui->autotimeEdit->text().toInt();
        if (ms < 100) {
            QMessageBox::warning(this, "time hint", "time should > 100ms");
            ui->autoCheckBox->setChecked(false);

            return;
        }

        // ������ʱ���¼�
        timerid = this->startTimer(ms);
    } else {
        // �رն�ʱ���¼�
        this->killTimer(timerid);
    }
}

// ��ʱ���¼�
void SerialPortApp::timerEvent(QTimerEvent *event) {
    on_sendBt_clicked();
}

// ����ѷ��͵��ֽ���
void SerialPortApp::on_clearSendSizeBt_clicked() {
    sendsize = 0;
    ui->sendText->clear();
    ui->sendsizelabel->setText("0");
}

// ���Ͷˣ�ʮ�����ƺ�ʮ����ת��
void SerialPortApp::on_sendHexCb_clicked(bool checked) {
    if (checked) {  // ʮ���� --> ʮ������
        QString data = ui->sendText->toPlainText();
        QByteArray array = data.toUtf8().toHex();
        ui->sendText->setText(QString(array));
    } else {  // ʮ������ --> ʮ����
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

// ���նˣ�ʮ�����ƺ�ʮ����ת��
void SerialPortApp::on_recvHexCb_clicked(bool checked) {
    if (checked) {  // ʮ���� --> ʮ������
        QString data = ui->recvText->toPlainText();
        QByteArray array = data.toUtf8().toHex();
        ui->recvText->setText(QString(array));
    } else {  // ʮ������ --> ʮ����
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

// ���մ�������
void SerialPortApp::read_data() {
    // ������������һ�����ֽ�
    QByteArray array = mSerial.readAll();
    recvsize += array.size();  // ��ʾ�ѽ��յ����ֽ���

    if (ui->recvHexCb->isChecked()) {
        ui->recvText->append(array.toHex());
    } else {
        ui->recvText->append(array);
    }

    // ������ʾ�ѽ��յ����ֽ���
    ui->recvsizelabel->setText(QString::number(recvsize));
}

// ����ѽ��յ��ֽ���
void SerialPortApp::on_clearRecvSizeBt_clicked() {
    recvsize = 0;
    ui->recvText->clear();
    ui->recvsizelabel->setText("0");
}

// ѡ��Ҫ���͵��ļ�
void SerialPortApp::on_selectfileBt_clicked() {
    QString path = QFileDialog::getOpenFileName(this);
    ui->filepathEdit->setText(path);
}

// �����ļ�
void SerialPortApp::on_sendfileBt_clicked() {
    // �����ݷ�����Ϻ�ᷢ��һ���ź� &QSerialPort::bytesWritten
    // ÿ����Ч�غɵ�����д�뵽�豸��ǰ��д��ͨ��ʱ���ͻᷢ������ź�
    connect(&mSerial, &QSerialPort::bytesWritten, this, &SerialPortApp::send_file_text);

    // ���ļ�
    file.setFileName(ui->filepathEdit->text());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    // ��ȡ�ļ���С
    int filesize = file.size();
    ui->progressBar->setMaximum(filesize);

    // ���ý�������ʾ
    QByteArray array = file.read(128);  // ÿ�ζ�ȡ 128 �ֽ�����
    sendfilesize = mSerial.write(array);
    ui->progressBar->setValue(sendfilesize);
}

// ѭ����ÿ 128 �ֽڣ������ļ�
void SerialPortApp::send_file_text(quint64 size) {
    // ���ý�������ʾ
    QByteArray array = file.read(128);
    quint64 mSize = mSerial.write(array);
    sendfilesize += mSize;
    ui->progressBar->setValue(sendfilesize);

    // �ж��ļ��Ƿ������
    if (sendfilesize == ui->progressBar->maximum()) {
        file.close();
        disconnect(&mSerial, &QSerialPort::bytesWritten, this, &SerialPortApp::send_file_text);
    }
}
