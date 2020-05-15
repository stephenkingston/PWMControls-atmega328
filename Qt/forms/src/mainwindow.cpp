#include "forms/headers/mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <include/serialportwriter.h>
#include <QList>
#include <QWidget>
#include <QFile>
#include <QTextStream>
QCPCurve *newCurve;
#include "forms/headers/tutorialdialog.h"
#include "forms/headers/about.h"
#include <fstream>

#define OC0A 0
#define OC0B 1
#define OC1A 2
#define OC1B 3
#define OC2A 4
#define OC2B 5

#define CTC_MODE 6
#define FAST_PWM 7
#define PHASE_CORRECTED 8

#define PRESCALER1 9
#define PRESCALER8 10
#define PRESCALER64 11
#define PRESCALER256 12
#define PRESCALER1024 13

#define SOFTWARE_BYTES 0
#define HARDWARE_BYTES 1
#define PC1 14
#define PC2 15
#define PC3 16

typedef struct
{
    uint8_t sample = 11;
    uint8_t sample2 = 22;
    uint8_t sample3 = 33;
}PWMSettings;

PWMSettings settings;
uint8_t write = 0;

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->graphWidget->setStyleSheet("border: 2px solid red");
    MainWindow::graphInit();
    QStringList pins = {"PC1", "PC2", "PC3"};
    QStringList PWMchannels = {"OC0A", "OC0B", "OC1A", "OC1B", "OC2A", "OC2B"};
    QStringList Prescaler = {"No prescaling", "Clock ÷ 8", "Clock ÷ 64", "Clock ÷ 256", "Clock ÷ 1024"};
    ui->ChannelCombo->addItems(pins);
    ui->fastPWMRadio->setChecked(true);
    ui->HardwareChannelsCombo->addItems(PWMchannels);
    ui->PrescalerCombo->addItems(Prescaler);
    ui->tabWidget->setCurrentIndex(0);
    checkCOM();
    ui->DutyCycleSlider->setValue(ui->Dut->value());
    ui->DutyCycleSlider_3->setValue(ui->Dut_3->value());
    graphChanged();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::checkCOM()
{
    ui->logBox->append("Checked COM Ports.");
    MainWindow::getSerialPorts();
}

void MainWindow::getSerialPorts()
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    ui->ChannelCombo_2->clear();
    int i = 0;
    for (const QSerialPortInfo &port : ports)
    {
        QString p = port.portName() + QObject::tr(", ") + port.description();
        ui->ChannelCombo_2->addItem(p);
        SerialPorts[i] = port.portName();
        i++;
    }
}

void MainWindow::frequencySpinUpdate(int value)
{
    ui->FrequencySpinBox->setValue(value);
    updateGraph(ui->FrequencySpinBox->value(), ui->Dut->value());
}


void MainWindow::frequencySliderUpdate(double arg1)
{
    if (arg1 <= 8000 && arg1 >=100)
        ui->FrequencySlider->setValue(arg1);
    else
        ui->FrequencySlider->setValue(100);
}

void MainWindow::dutySpinUpdate(int value)
{
    ui->Dut->setValue(value);
    updateGraph(ui->FrequencySpinBox->value(), ui->Dut->value());
}

void MainWindow::dutySliderUpdate(double value)
{
    if (value <= 100 && value >=0)
        ui->DutyCycleSlider->setValue(value);
    else
        ui->DutyCycleSlider->setValue(50);
}

void MainWindow::dutySliderHardwareUpdate(double value)
{
    if (value <= 100 && value >=0)
        ui->DutyCycleSlider_3->setValue(value);
    else
        ui->DutyCycleSlider_3->setValue(50);
}

void MainWindow::dutySpinUpdateHardware(int value)
{
    ui->Dut_3->setValue(value);
    float duty;
    if (ui->CTCRadioButton->isChecked())
        duty = 50;
    else
        duty = ui->Dut_3->value();
    updateGraph(getHardwareFrequency(), duty);
}

void MainWindow::flagAll(bool b)
{
    if (b)
    {
        PPS.inversion = 1;
    }
    else
    {
        PPS.inversion = 0;
    }
    updateGraph(ui->FrequencySpinBox->value(), ui->Dut->value());
}

void MainWindow::setFlagHardware(bool b)
{
    if (b)
    {
        PPH.inversion = 1;
    }
    else
    {
        PPH.inversion = 0;
    }
    updateGraph(getHardwareFrequency(), ui->Dut_3->value());
}

void MainWindow::graphInit()
{
    ui->graphWidget->addGraph();
    ui->graphWidget->xAxis->setLabel("Time (ms)");
    ui->graphWidget->yAxis->setLabel("Logic Level");
    ui->graphWidget->xAxis->setRange(0, 3);
    ui->graphWidget->yAxis->setRange(-0.2, 1.2);
    ui->graphWidget->yAxis->setTicks(false);
    ui->graphWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    newCurve = new QCPCurve(ui->graphWidget->xAxis, ui->graphWidget->yAxis);
    updateGraph(ui->FrequencySpinBox->value(), ui->Dut->value());

    ui->graphWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    ui->graphWidget->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->graphWidget->axisRect()->setRangeDragAxes(ui->graphWidget->xAxis, 0);
    ui->graphWidget->axisRect()->setRangeZoomAxes(ui->graphWidget->xAxis, 0);

}

void MainWindow::updateGraph(float frequency, float dutyCycle)
{
    uint8_t inversion = 0;
    if (ui->tabWidget->currentIndex() == 0)
        inversion = PPS.inversion;
    else
        inversion = PPH.inversion;
    QVector<double> x(1000), y(1000);
    x[0] = 0.001; y[0] = 1-inversion;

    float wavelength = (1/frequency)*1000; //wavelength in ms

    x[1] = (wavelength) * dutyCycle/100;
    y[1] = 1-inversion;
    x[2] = x[1];
    y[2] = 0+inversion;
    x[3] = wavelength;
    y[3] = 0+inversion;
    int i = 4;
    while(i<997)
    {
        x[i] = wavelength*((i/4));
        y[i] = 1-inversion;
        x[i+1] = (wavelength * dutyCycle/100)+x[i];
        y[i+1] = 1-inversion;
        x[i+2] = x[i+1];
        y[i+2] = 0+inversion;
        x[i+3] = wavelength+x[i];
        y[i+3] = 0+inversion;
        i+=4;
    }
    newCurve->data().data()->clear();
    newCurve->setData(x, y);
    QString status = QString::fromStdString("Frequency: " + std::to_string(frequency/1000)+ " KHz" + "   Period: " + std::to_string(wavelength) + " ms");
    ui->statusLabel->setText(status);
    ui->graphWidget->replot();
}

void MainWindow::sendToMicroHardware()
{
    try
    {
        write = 1;
        QString port = getSelectedSerialPort();
        QSerialPort SerialPort;
        SerialPort.setPortName(port);
        SerialPort.setBaudRate(QSerialPort::Baud9600);
        SerialPort.open(QIODevice::WriteOnly);
        SerialPort.setDataBits(QSerialPort::Data8);
        SerialPort.setStopBits(QSerialPort::OneStop);

        writeSettingsToHardwareStruct();
        QByteArray hardwareBytes = getByteArray(HARDWARE_BYTES);
        SerialPortWriter serialPortWriter(&SerialPort);
        serialPortWriter.write(hardwareBytes);

        SerialPort.flush();
        writeLog(ui->FrequencySpinBox->value(), ui->Dut_3->value());
        write = 0;
        SerialPort.close();
    }
    catch (std::string e)
    {
        ui->logBox->append("Error writing to serial port." + QString::fromStdString(e));
    }
}

void MainWindow::writeSettingsToSoftwareStruct()
{
    if(ui->PWMCheck->isChecked())
        PPS.inversion = 1;
    else
        PPS.inversion = 0;

    PPS.pin = (ui->ChannelCombo->currentIndex()) + 14;

    PPS.PWM_DutyCycle = ui->Dut->value();
    uint16_t freq = ui->FrequencySpinBox->value();

    PPS.FrequencyA = *((uint8_t*)&freq);
    PPS.FrequencyB = *((uint8_t*)&freq + 1);
}

void MainWindow::writeSettingsToHardwareStruct()
{
    if (ui->PWMCheck_2->isChecked())
        PPH.inversion = 1;
    else
        PPH.inversion = 0;

    PPH.pin = ui->HardwareChannelsCombo->currentIndex();

    if(ui->CTCRadioButton->isChecked())
        PPH.PWM_Mode = CTC_MODE;
    else if (ui->fastPWMRadio->isChecked())
        PPH.PWM_Mode = FAST_PWM;
    else
        PPH.PWM_Mode = PHASE_CORRECTED;
    PPH.PWM_DutyCycle = ui->Dut_3->value();

    PPH.PWM_frequency_index =  (ui->PrescalerCombo->currentIndex()) + PRESCALER1;
}

QString MainWindow::getSelectedSerialPort()
{
    activePort = SerialPorts[ui->ChannelCombo_2->currentIndex()];
    return activePort;
}

QByteArray MainWindow::getByteArray(uint8_t tabIndex)
{
    uint8_t HS_flag = ui->tabWidget->currentIndex();
    if (tabIndex == 0)
    {
        QByteArray softwareBytes;
        if (write == 1) softwareBytes.append(HS_flag);
        softwareBytes.append(PPS.pin);
        softwareBytes.append(PPS.PWM_DutyCycle);
        softwareBytes.append(PPS.inversion);
        softwareBytes.append(PPS.FrequencyA);
        softwareBytes.append(PPS.FrequencyB);
        return softwareBytes;
    }
    else
    {
        QByteArray hardwareBytes;
        if (write == 1) hardwareBytes.append(HS_flag);
        hardwareBytes.append(PPH.pin);
        hardwareBytes.append(PPH.PWM_Mode);
        hardwareBytes.append(PPH.inversion);
        hardwareBytes.append(PPH.PWM_DutyCycle);
        hardwareBytes.append(PPH.PWM_frequency_index);
        return hardwareBytes;
    }
}
void MainWindow::sendToMicroSoftware()
{
    try
    {
        write = 1;
        QString port = getSelectedSerialPort();
        QSerialPort SerialPort;
        SerialPort.setPortName(port);
        SerialPort.open(QIODevice::WriteOnly);
        SerialPort.setBaudRate(QSerialPort::Baud9600);
        SerialPort.setDataBits(QSerialPort::Data8);
        SerialPort.setStopBits(QSerialPort::OneStop);
        writeSettingsToSoftwareStruct();
        QByteArray softwareBytes = getByteArray(SOFTWARE_BYTES);

        SerialPortWriter writer(&SerialPort);
        writer.write(softwareBytes);
        SerialPort.flush();
        writeLog(ui->FrequencySpinBox->value(), ui->Dut->value());
        write = 0;
        SerialPort.close();
    }
    catch (std::string e)
    {
        ui->logBox->append("Error writing to serial port." + QString::fromStdString(e));
    }

}

void MainWindow::writeLog(double f, double d)
{
    std::string output;
    QString q_output;
    output = "Wrote PWM settings to microcontroller: Frequency: " + std::to_string((int)f) + "   Duty Cycle: " + std::to_string((int)d)+"% (";
    if (ui->tabWidget->currentIndex() == 0)
        q_output = QString::fromStdString(output) + (ui->ChannelCombo->currentText()) + QString::fromStdString(")");
    else
        q_output = QString::fromStdString(output) + (ui->HardwareChannelsCombo->currentText()) + QString::fromStdString(")");
    ui->logBox->append(q_output);
}

void MainWindow::HardwareTabGraph(int tabIndex)
{
    if (tabIndex == 1)
    {
        float f = getHardwareFrequency();
        float duty;
        if (ui->CTCRadioButton->isChecked())
            duty = 50;
        else
            duty = ui->Dut_3->value();
        updateGraph(f, duty);
    }
    else
    {
        updateGraph(ui->FrequencySpinBox->value(), ui->Dut->value());
    }
}

float MainWindow::getHardwareFrequency()
{
    float uCf = (ui->uCfSpin->value())*1000000;
    int p = 1;
    float f = 1;

    switch(ui->PrescalerCombo->currentIndex())
    {
        case 0:
            p = 1;
        break;
        case 1:
            p = 8;
        break;
        case 2:
            p = 64;
        break;
        case 3:
            p = 256;
        break;
        case 4:
            p = 1024;
        break;
        default:
            p = 1;
        break;
    };


    if (ui->fastPWMRadio->isChecked())
    {
        f = uCf/(p*256);
        return f;
    }
    else if(ui->CTCRadioButton->isChecked())
    {
        uint16_t ocr = (ui->Dut_3->value()/100)*256;
        f = uCf/(2*p*(1 + ocr));
        return f;
    }
    else if(ui->PhaseCorrectPWM->isChecked())
    {
        f = uCf/(p*256*2);
        return f;
    }
    else
        return f;
}

void MainWindow::graphChanged()
{
    if(ui->CTCRadioButton->isChecked())
    {
        ui->DutyCycleGroup_2->setTitle("OCR [0-100]");
    }
   else
    {
        ui->DutyCycleGroup_2->setTitle("Duty Cycle (%)");
    }
    if(ui->tabWidget->currentIndex() == 1)
        HardwareTabGraph(1);
    else
        HardwareTabGraph(0);
}

void MainWindow::graphChangedproxy(int index)
{
    graphChanged();
    index = 0;
}

void MainWindow::graphChangedproxy(double frequency)
{
    graphChanged();
    frequency = 0;
    PPH.uF = ui->uCfSpin->value();
}

void MainWindow::on_actionTutorial_triggered()
{
    TutDialog = new tutorialDialog(this);
    TutDialog->show();
}

void MainWindow::on_actionAbout_triggered()
{
    aboutDialog = new About(this);
    aboutDialog->show();
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
           tr("Save PWM Settings"), "",
           tr("PWM Settings File (*.pwms);;All Files (*)"));

    if (fileName.isEmpty())
           return;
    else
    {
           QFile file(fileName);
           if (!file.open(QIODevice::WriteOnly))
           {
               QMessageBox::information(this, tr("Unable to write to file!"),
                   file.errorString());
               return;
           }
           QDataStream out(&file);
           writeSettingsToHardwareStruct();
           writeSettingsToSoftwareStruct();

           QByteArray softwareBytes = getByteArray(SOFTWARE_BYTES);
           QByteArray hardwareBytes = getByteArray(HARDWARE_BYTES);

           QByteArray clock_frequency(reinterpret_cast<const char*>(&PPH.uF), sizeof(PPH.uF));
           out << softwareBytes + hardwareBytes + clock_frequency;
           ui->logBox->append("Saved file to " + fileName);
    }
}

void MainWindow::on_actionLoad_triggered()
{
      QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open PWM Settings File"), "",
            tr("PWM Settings File (*.pwms);;All Files (*)"));

      if (fileName.isEmpty())
              return;
      else
      {
              QFile file(fileName);

              if (!file.open(QIODevice::ReadOnly))
              {
                  QMessageBox::information(this, tr("Unable to open file"),
                      file.errorString());
                  return;
              }
              QDataStream in(&file);
              QByteArray savedFile;
              in >> savedFile;
              char* pointer = savedFile.data();
              loadFile((uint8_t*)pointer);
              ui->logBox->append("Loaded savefile from " + fileName);
      }
}

void MainWindow::loadFile(uint8_t* config)
{
    PPS.pin = config[0];
    PPS.PWM_DutyCycle = config[1];
    PPS.inversion = config[2];
    PPS.FrequencyA = config[3];
    PPS.FrequencyB = config[4];

    PPH.pin = config[5];
    PPH.PWM_Mode = config[6];
    PPH.inversion = config[7];
    PPH.PWM_DutyCycle = config[8];
    PPH.PWM_frequency_index = config[9];

    *((uint8_t*)&PPS.Frequency) = PPS.FrequencyA;
    *(((uint8_t*)&PPS.Frequency)+1) = PPS.FrequencyB;

    PPH.uF = *((float*)(config + 10));
    reflectStructToUI();
}

void MainWindow::reflectStructToUI()
{
    ui->ChannelCombo->setCurrentIndex(PPS.pin-PC1);
    ui->Dut->setValue(PPS.PWM_DutyCycle);
    ui->FrequencySpinBox->setValue(PPS.Frequency);
    if (PPS.inversion == 1) ui->PWMCheck->setCheckState(Qt::Checked);
    else ui->PWMCheck->setCheckState(Qt::Unchecked);

    ui->HardwareChannelsCombo->setCurrentIndex(PPH.pin);
    if(PPH.PWM_Mode == CTC_MODE)  ui->CTCRadioButton->setChecked(true);
    else if (PPH.PWM_Mode == FAST_PWM) ui->fastPWMRadio->setChecked(true);
    else ui->PhaseCorrectPWM->setChecked(true);
    if (PPH.inversion == 1) ui->PWMCheck_2->setChecked(true);
    else ui->PWMCheck_2->setChecked(false);
    ui->Dut_3->setValue(PPH.PWM_DutyCycle);
    ui->PrescalerCombo->setCurrentIndex(PPH.PWM_frequency_index-PRESCALER1);
    ui->uCfSpin->setValue(PPH.uF);
}
