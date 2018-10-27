#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_InDataReady();
    void on_Out1DataReady();
    void on_Out2DataReady();
    void on_pbStart_clicked();
    void on_pbStop_clicked();

    void on_cbxIn2Out1_stateChanged(int arg1);
    void on_cbxIn2Out2_stateChanged(int arg1);
    void on_cbxOut1ToIn_stateChanged(int arg1);
    void on_cbxOut2ToIn_stateChanged(int arg1);

    void on_needUpdGui();

    void on_cbDumpIn_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

    void FillGui();

    QList<QSerialPort::DataBits>     m_dataBits;
    QList<QSerialPort::FlowControl>  m_flow;
    QList<QSerialPort::Parity>       m_parity;
    QList<QSerialPort::StopBits>     m_stopBits;
    QList<qint32> m_baud;

    QStringList m_str_dataBits;
    QStringList m_str_flow;
    QStringList m_str_parity;
    QStringList m_str_stopBits;

    void InitLists();

    QSerialPort m_portIn, m_portOut1, m_portOut2;

    bool Start();
    bool Stop();

    bool i_o1, i_o2, o1_i, o2_i;
    bool m_started;

    QTimer m_guiTimer;

    quint64 m_cnt_i_rd, m_cnt_o1_rd, m_cnt_o2_rd;
    quint64 m_cnt_i_rd_old, m_cnt_o1_rd_old, m_cnt_o2_rd_old;

    quint64 m_cnt_i_wr, m_cnt_o1_wr, m_cnt_o2_wr;
    quint64 m_cnt_i_wr_old, m_cnt_o1_wr_old, m_cnt_o2_wr_old;

    quint64 m_cnt_i_wr_err, m_cnt_o1_wr_err, m_cnt_o2_wr_err;

    QElapsedTimer m_elapsed;

    QFile m_f_In_dump;
    bool OpenF();
};

#endif // MAINWINDOW_H
