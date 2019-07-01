#include "widget.h"
#include "ui_widget.h"
#include <unistd.h>
#include <QTimer>
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->BaudRateBox->addItem(QLatin1String("9600"), QSerialPort::Baud9600);
    ui->BaudRateBox->addItem(QLatin1String("19200"), QSerialPort::Baud19200);
    ui->BaudRateBox->addItem(QLatin1String("38400"), QSerialPort::Baud38400);
    ui->BaudRateBox->addItem(QLatin1String("115200"), QSerialPort::Baud115200);
   // fill data bits
    ui->DataBitsBox->addItem(QLatin1String("5"), QSerialPort::Data5);
    ui->DataBitsBox->addItem(QLatin1String("6"), QSerialPort::Data6);
    ui->DataBitsBox->addItem(QLatin1String("7"), QSerialPort::Data7);
    ui->DataBitsBox->addItem(QLatin1String("8"), QSerialPort::Data8);
    ui->DataBitsBox->setCurrentIndex(3);
   // fill parity
    ui->ParityBox->addItem(QLatin1String("None"), QSerialPort::NoParity);
    ui->ParityBox->addItem(QLatin1String("Even"), QSerialPort::EvenParity);
    ui->ParityBox->addItem(QLatin1String("Odd"), QSerialPort::OddParity);
    ui->ParityBox->addItem(QLatin1String("Mark"), QSerialPort::MarkParity);
    ui->ParityBox->addItem(QLatin1String("Space"), QSerialPort::SpaceParity);
    ui->PortNameBox->clear();
    QTimer* tmr=new QTimer;
    tmr->start(1000);
    connect(tmr, &QTimer::timeout, [=](){
        ui->PortNameBox->clear();
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            ui->PortNameBox->addItem(info.portName());
        }
        if(ui->PortNameBox->count()==0){
            ui->textEdit->setText("No connections available");
            ui->ButtonConnect->setDisabled(true);
        }
        else{
            ui->ButtonConnect->setDisabled(false);
        }
    });
    serialport = new QSerialPort();
    QObject::connect(serialport,&QSerialPort::errorOccurred, [=](QSerialPort::SerialPortError error){
        if ( (serialport->isOpen()) && (error == QSerialPort::ResourceError)) {
            ui->textEdit->append("Port Status Error : "+serialport->errorString());
            serialport->close();
            ui->ButtonConnect->setText("Connect");
        }
    });
    QObject::connect(ui->ButtonConnect,SIGNAL(clicked(bool)),this,SLOT(on_pushButton_clicked()));
    QObject::connect(serialport,&QSerialPort::readyRead, [=](){
        QByteArray ba;
        ba.append(serialport->readAll());
        QDataStream stream(&ba,  QIODevice::ReadOnly);
        int pref=0;
        float x=0;
        float y=0;
        float z=0;
        stream>>pref>>x>>y>>z;
        if(pref==0x40){
            ui->textEdit->append("Float x="+QString::number(x)+"; Float y="+QString::number(y)+"; Float z="+QString::number(z)+";");
        }
        else{
            ui->textEdit->append("Wrong package format");
        }
        serialport->flush();
    });
}

Widget::~Widget()
{
    delete ui;
    delete serialport;
}

void Widget::on_pushButton_clicked()
{
    if(serialport->isOpen()){
        serialport->flush();
        serialport->close();
        ui->textEdit->append(serialport->portName()+">>Close...");
        ui->ButtonConnect->setText("Connect");
    }
    else{
        serialport->setPortName(ui->PortNameBox->currentText());
        serialport->setBaudRate((QSerialPort::BaudRate) ui->BaudRateBox->currentText().toInt());
        serialport->setDataBits((QSerialPort::DataBits) ui->DataBitsBox->currentText().toInt());
        serialport->setParity((QSerialPort::Parity) ui->ParityBox->currentText().toInt());
        if(serialport->open(QIODevice::ReadWrite)){
            QByteArray ba;
            QDataStream stream(&ba, QIODevice::WriteOnly);
            int pref=0x40;
            float x=1.2;
            float y=2.3;
            float z=3.3;
            stream<<pref<<x<<y<<z;
            serialport->write(ba,ba.size());
            ui->textEdit->append(serialport->portName()+">>Open...");
            ui->ButtonConnect->setText("Disconnect");
        }
        else{
            ui->textEdit->append("Can not connect to port : "+serialport->errorString());
        }
    }
}
