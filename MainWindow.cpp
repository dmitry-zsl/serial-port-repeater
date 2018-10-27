#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QComboBox>
#include <QSettings>
#include <QDir>
#include <QCheckBox>
#include <QMessageBox>
#include <QDebug>
#include <QSignalBlocker>
#include <QLocale>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),m_cnt_i_rd(0), m_cnt_o1_rd(0), m_cnt_o2_rd(0),
    m_cnt_i_rd_old(0), m_cnt_o1_rd_old(0), m_cnt_o2_rd_old(0),
    m_cnt_i_wr(0), m_cnt_o1_wr(0), m_cnt_o2_wr(0),
    m_cnt_i_wr_old(0), m_cnt_o1_wr_old(0), m_cnt_o2_wr_old(0),
    m_cnt_i_wr_err(0), m_cnt_o1_wr_err(0), m_cnt_o2_wr_err(0)
{
    ui->setupUi(this);

    InitLists();
    FillGui();

    QSettings sett(QDir::currentPath()+"/settings.ini", QSettings::IniFormat);
    QList<QComboBox*> cbList = this->findChildren<QComboBox *>();
    QComboBox* cb;
    foreach (cb, cbList) {
        int i = sett.value( cb->objectName(), 0 ).toInt();
        if (i>=0 && i<cb->count()) cb->setCurrentIndex(i);
    }

    QList<QCheckBox*> cbxList = this->findChildren<QCheckBox *>();
    QCheckBox* cbx;
    foreach (cbx, cbxList) {
        const QSignalBlocker blocker(cbx);
        bool checked = sett.value( cbx->objectName(), true).toBool();
        cbx->setChecked(checked);
    }

    connect( &m_portIn  , SIGNAL(readyRead()), this, SLOT(on_InDataReady()  ));
    connect( &m_portOut1, SIGNAL(readyRead()), this, SLOT(on_Out1DataReady()));
    connect( &m_portOut2, SIGNAL(readyRead()), this, SLOT(on_Out2DataReady()));

    m_started = false;

    connect(&m_guiTimer, SIGNAL(timeout()), this, SLOT(on_needUpdGui()));
    m_guiTimer.start(500);
}

MainWindow::~MainWindow()
{
    QSettings sett(QDir::currentPath()+"/settings.ini", QSettings::IniFormat);
    QList<QComboBox*> cbList = this->findChildren<QComboBox *>();
    QComboBox* cb;
    foreach (cb, cbList) {
        sett.setValue( cb->objectName(), cb->currentIndex() );
    }
    QList<QCheckBox*> cbxList = this->findChildren<QCheckBox *>();
    QCheckBox* cbx;
    foreach (cbx, cbxList) {
        sett.setValue( cbx->objectName(), cbx->isChecked());
    }
    Stop();
    delete ui;
}

void MainWindow::on_InDataReady()
{
    while (m_portIn.bytesAvailable()) {
        QByteArray data = m_portIn.readAll();
        m_cnt_i_rd += data.size();
        if (i_o1 && m_portOut1.isOpen()) {
            qint64 ws = m_portOut1.write(data);
            if (ws>-1) m_cnt_o1_wr += ws;
            else ++m_cnt_o1_wr_err;
        }
        if (i_o2 && m_portOut2.isOpen()) {
            qint64 ws = m_portOut2.write(data);
            if (ws>-1) m_cnt_o2_wr += ws;
            else ++m_cnt_o1_wr_err;
        }
        if (m_f_In_dump.isOpen()) {
            m_f_In_dump.write(data);
        }
    }
}

void MainWindow::on_Out1DataReady()
{
    while (m_portOut1.bytesAvailable()) {
        QByteArray data = m_portOut1.readAll();
        m_cnt_o1_rd += data.size();
        if (o1_i && m_portIn.isOpen()) {
            qint64 ws = m_portIn.write(data);
            if (ws>-1) m_cnt_i_wr += ws;
            else ++m_cnt_i_wr_err;
        }
    }
}

void MainWindow::on_Out2DataReady()
{
    while (m_portOut2.bytesAvailable()) {
        QByteArray data = m_portOut2.readAll();
        m_cnt_o2_rd += data.size();
        if (o2_i && m_portIn.isOpen()) {
            qint64 ws = m_portIn.write(data);
            if (ws>-1) m_cnt_i_wr += ws;
            else ++m_cnt_i_wr_err;
        }
    }
}

void MainWindow::FillGui()
{
    QStringList sl;
    foreach (const QSerialPortInfo &entry, QSerialPortInfo::availablePorts() ) {
        sl.push_back(entry.portName());
    }    
    ui->cbInPort->addItems(sl);
    ui->cbOut1Port->addItems(sl);
    ui->cbOut2Port->addItems(sl);

    m_baud.push_back(110);
    m_baud.push_back(150);
    m_baud.push_back(300);
    m_baud.push_back(1200);
    m_baud.push_back(2400);
    m_baud.push_back(4800);
    m_baud.push_back(9600);
    m_baud.push_back(19200);
    m_baud.push_back(38400);
    m_baud.push_back(57600);
    m_baud.push_back(115200);
    m_baud.push_back(230400);
    m_baud.push_back(460800);
    m_baud.push_back(921600);
    sl.clear();
    for (auto it=m_baud.begin(); it!=m_baud.end(); ++it) {
        sl.push_back(QString::number(*it));
    }
    ui->cbInBaud->addItems(sl);
    ui->cbOut1Baud->addItems(sl);
    ui->cbOut2Baud->addItems(sl);

    ui->cbInDataBits->addItems(m_str_dataBits);
    ui->cbOut1DataBits->addItems(m_str_dataBits);
    ui->cbOut2DataBits->addItems(m_str_dataBits);

    ui->cbInFlow->addItems(m_str_flow);
    ui->cbOut1Flow->addItems(m_str_flow);
    ui->cbOut2Flow->addItems(m_str_flow);

    ui->cbInParity->addItems(m_str_parity);
    ui->cbOut1Parity->addItems(m_str_parity);
    ui->cbOut2Parity->addItems(m_str_parity);

    ui->cbInStopBits->addItems(m_str_stopBits);
    ui->cbOut1StopBits->addItems(m_str_stopBits);
    ui->cbOut2StopBits->addItems(m_str_stopBits);

}

void MainWindow::InitLists()
{
    m_dataBits.clear();
    m_flow.clear();
    m_parity.clear();
    m_stopBits.clear();

    m_str_dataBits.clear();
    m_str_flow.clear();
    m_str_parity.clear();
    m_str_stopBits.clear();

    m_dataBits.push_back(QSerialPort::Data5);
    m_dataBits.push_back(QSerialPort::Data6);
    m_dataBits.push_back(QSerialPort::Data7);
    m_dataBits.push_back(QSerialPort::Data8);
    m_str_dataBits.push_back(tr("5"));
    m_str_dataBits.push_back(tr("6"));
    m_str_dataBits.push_back(tr("7"));
    m_str_dataBits.push_back(tr("8"));

    m_flow.push_back(QSerialPort::NoFlowControl);
    m_flow.push_back(QSerialPort::HardwareControl);
    m_flow.push_back(QSerialPort::SoftwareControl);
    m_str_flow.push_back(tr("No control"));
    m_str_flow.push_back(tr("Hardware"));
    m_str_flow.push_back(tr("Software"));

    m_parity.push_back(QSerialPort::NoParity);
    m_parity.push_back(QSerialPort::EvenParity);
    m_parity.push_back(QSerialPort::OddParity);
    m_parity.push_back(QSerialPort::SpaceParity);
    m_parity.push_back(QSerialPort::MarkParity);
    m_str_parity.push_back(tr("No parity"));
    m_str_parity.push_back(tr("Even"));
    m_str_parity.push_back(tr("Odd"));
    m_str_parity.push_back(tr("Space"));
    m_str_parity.push_back(tr("Mark"));

    m_stopBits.push_back(QSerialPort::OneStop);
    m_stopBits.push_back(QSerialPort::OneAndHalfStop);
    m_stopBits.push_back(QSerialPort::TwoStop);
    m_str_stopBits.push_back(tr("1"));
    m_str_stopBits.push_back(tr("1.5"));
    m_str_stopBits.push_back(tr("2"));
}

bool MainWindow::Start()
{
    m_portIn.setPortName( ui->cbInPort->currentText() );
    m_portIn.setBaudRate( m_baud[ui->cbInBaud->currentIndex()] );
    m_portIn.setDataBits( m_dataBits[ui->cbInDataBits->currentIndex()]);
    m_portIn.setParity  ( m_parity[ui->cbInParity->currentIndex()]);
    m_portIn.setFlowControl( m_flow[ui->cbInFlow->currentIndex()]);
    m_portIn.setStopBits( m_stopBits[ui->cbInStopBits->currentIndex()]);

    m_portOut1.setPortName( ui->cbOut1Port->currentText() );
    m_portOut1.setBaudRate( m_baud[ui->cbOut1Baud->currentIndex()] );
    m_portOut1.setDataBits( m_dataBits[ui->cbOut1DataBits->currentIndex()]);
    m_portOut1.setParity  ( m_parity[ui->cbOut1Parity->currentIndex()]);
    m_portOut1.setFlowControl( m_flow[ui->cbOut1Flow->currentIndex()]);
    m_portOut1.setStopBits( m_stopBits[ui->cbOut1StopBits->currentIndex()]);

    m_portOut2.setPortName( ui->cbOut2Port->currentText() );
    m_portOut2.setBaudRate( m_baud[ui->cbOut2Baud->currentIndex()] );
    m_portOut2.setDataBits( m_dataBits[ui->cbOut2DataBits->currentIndex()]);
    m_portOut2.setParity  ( m_parity[ui->cbOut2Parity->currentIndex()]);
    m_portOut2.setFlowControl( m_flow[ui->cbOut2Flow->currentIndex()]);
    m_portOut2.setStopBits( m_stopBits[ui->cbOut2StopBits->currentIndex()]);

    i_o1 = ui->cbxIn2Out1->isChecked();
    i_o2 = ui->cbxIn2Out2->isChecked();
    o1_i = ui->cbxOut1ToIn->isChecked();
    o2_i = ui->cbxOut2ToIn->isChecked();

    if (!m_portIn.open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, tr("Error"), tr("Can't open port %1").arg(m_portIn.portName()), QMessageBox::Ok);
        Stop();
        return false;
    }
    if (i_o1 && o1_i) {
        if (!m_portOut1.open(QIODevice::ReadWrite)) {
            QMessageBox::critical(this, tr("Error"), tr("Can't open port %1").arg(m_portOut1.portName()), QMessageBox::Ok);
            Stop();
            return false;
        }
    }
    if (i_o2 && o2_i) {
        if (!m_portOut2.open(QIODevice::ReadWrite)) {
            QMessageBox::critical(this, tr("Error"), tr("Can't open port %1").arg(m_portOut2.portName()), QMessageBox::Ok);
            Stop();
            return false;
        }
    }
    m_started = true;
    m_cnt_i_rd=m_cnt_o1_rd=m_cnt_o2_rd=0;
    m_cnt_i_rd_old=m_cnt_o1_rd_old=m_cnt_o2_rd_old=0;
    m_cnt_i_wr=m_cnt_o1_wr=m_cnt_o2_wr=0;
    m_cnt_i_wr_old=m_cnt_o1_wr_old=m_cnt_o2_wr_old=0;
    m_cnt_i_wr_err=m_cnt_o1_wr_err=m_cnt_o2_wr_err=0;
    if (ui->cbDumpIn->isChecked()) {
        if (!OpenF()) {
            QMessageBox::critical(this, tr("Error"), tr("Can't open dump file for write"), QMessageBox::Ok);
        }
    }
    return true;
}

bool MainWindow::Stop()
{
    if (m_portIn.isOpen()  ) m_portIn.close();
    if (m_portOut1.isOpen()) m_portOut1.close();
    if (m_portOut2.isOpen()) m_portOut2.close();
    m_started = false;
    if (m_f_In_dump.isOpen()) {
        m_f_In_dump.flush();
        m_f_In_dump.close();
    }
    return true;
}

bool MainWindow::OpenF()
{
    QString dir = QDir::currentPath() + "/dumps";
    QDir d(dir);
    if (!d.exists(dir)) {
        d.mkdir(dir);
    }
    dir += "/" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".dump";
    m_f_In_dump.setFileName(dir);
    if (m_f_In_dump.open(QIODevice::WriteOnly)) {
        return true;
    } else {
        return false;
    }
}


void MainWindow::on_pbStart_clicked()
{
    Start();
}

void MainWindow::on_pbStop_clicked()
{
    Stop();
}

void MainWindow::on_cbxIn2Out1_stateChanged(int arg1)
{
    i_o1 = arg1;
    if (m_started) {
        if (i_o1 && !m_portOut1.isOpen()) {
            if (!m_portOut1.open(QIODevice::ReadWrite)) {
                QMessageBox::critical(this, tr("Error"), tr("Can't open %1").arg(m_portOut1.portName()), QMessageBox::Ok);
                i_o1 = false;
                ui->cbxIn2Out1->setChecked(false);
            }
        }
    }
}

void MainWindow::on_cbxIn2Out2_stateChanged(int arg1)
{
    i_o2 = arg1;
    if (m_started) {
        if (i_o2 && !m_portOut2.isOpen()) {
            if (!m_portOut2.open(QIODevice::ReadWrite)) {
                QMessageBox::critical(this, tr("Error"), tr("Can't open %1").arg(m_portOut2.portName()), QMessageBox::Ok);
                i_o2 = false;
                ui->cbxIn2Out2->setChecked(false);
            }
        }
    }
}

void MainWindow::on_cbxOut1ToIn_stateChanged(int arg1)
{
    o1_i = arg1;
    if (m_started) {
        if (o1_i && !m_portOut1.isOpen()) {
            if (!m_portOut1.open(QIODevice::ReadWrite)) {
                QMessageBox::critical(this, tr("Error"), tr("Can't open %1").arg(m_portOut1.portName()), QMessageBox::Ok);
                o1_i = false;
                ui->cbxIn2Out1->setChecked(false);
            }
        }
    }
}

void MainWindow::on_cbxOut2ToIn_stateChanged(int arg1)
{
    o2_i = arg1;
    if (m_started) {
        if (o2_i && !m_portOut2.isOpen()) {
            if (!m_portOut2.open(QIODevice::ReadWrite)) {
                QMessageBox::critical(this, tr("Error"), tr("Can't open %1").arg(m_portOut2.portName()), QMessageBox::Ok);
                o2_i = false;
                ui->cbxIn2Out2->setChecked(false);
            }
        }
    }
}

void MainWindow::on_needUpdGui()
{
    double elapsed = m_elapsed.restart()*1e-3;
    double f1 = (m_cnt_i_rd - m_cnt_i_rd_old)/elapsed;
    double f2 = (m_cnt_o1_rd - m_cnt_o1_rd_old)/elapsed;
    double f3 = (m_cnt_o2_rd - m_cnt_o2_rd_old)/elapsed;
    m_cnt_i_rd_old  = m_cnt_i_rd;
    m_cnt_o1_rd_old = m_cnt_o1_rd;
    m_cnt_o2_rd_old = m_cnt_o2_rd;

    double fw1 = (m_cnt_i_wr - m_cnt_i_wr_old)/elapsed;
    double fw2 = (m_cnt_o1_wr - m_cnt_o1_wr_old)/elapsed;
    double fw3 = (m_cnt_o2_wr - m_cnt_o2_wr_old)/elapsed;
    m_cnt_i_wr_old  = m_cnt_i_wr;
    m_cnt_o1_wr_old = m_cnt_o1_wr;
    m_cnt_o2_wr_old = m_cnt_o2_wr;

    QLocale currentLocale = QLocale::system();
    ui->lblInBytesReaded->setText(currentLocale.toString(m_cnt_i_rd));
    ui->lblOut1BytesReaded->setText(currentLocale.toString(m_cnt_o1_rd));
    ui->lblOut2BytesReaded->setText(currentLocale.toString(m_cnt_o2_rd));

    ui->lblInSpeed->setText(currentLocale.toString(f1));
    ui->lblOut1Speed->setText(currentLocale.toString(f2));
    ui->lblOut2Speed->setText(currentLocale.toString(f3));

    ui->lblInBytesWritten->setText(currentLocale.toString(m_cnt_i_wr));
    ui->lblOut1BytesWritten->setText(currentLocale.toString(m_cnt_o1_wr));
    ui->lblOut2BytesWritten->setText(currentLocale.toString(m_cnt_o2_wr));

    ui->lblInWriteSpeed->setText(currentLocale.toString(fw1));
    ui->lblOut1WriteSpeed->setText(currentLocale.toString(fw2));
    ui->lblOut2WriteSpeed->setText(currentLocale.toString(fw3));

    ui->lblInWriteErrors->setText(currentLocale.toString(m_cnt_i_wr_err));
    ui->lblOut1WriteErrors->setText(currentLocale.toString(m_cnt_o1_wr_err));
    ui->lblOut2WriteErrors->setText(currentLocale.toString(m_cnt_o2_wr_err));
}

void MainWindow::on_cbDumpIn_stateChanged(int arg1)
{
    if (arg1 && !m_f_In_dump.isOpen()) {
        if (!OpenF()) {
            QMessageBox::critical(this, tr("Error"), tr("Can't open dump file for write"), QMessageBox::Ok);
        }
    }
    if (!arg1) {
        m_f_In_dump.flush();
        m_f_In_dump.close();
    }
}
