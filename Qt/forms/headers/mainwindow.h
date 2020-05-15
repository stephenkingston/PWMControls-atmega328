#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFont>
#include "tutorialdialog.h"
#include "about.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void getSerialPorts();
    void graphInit();
    void updateGraph(float frequency, float dutyCycle);
    void writeLog(double f, double d);
    float getHardwareFrequency();
    QString getSelectedSerialPort();
    void writeSettingsToSoftwareStruct();
    void writeSettingsToHardwareStruct();
    QByteArray getByteArray(uint8_t tabIndex);
    void loadFile(uint8_t* pointer);
    void reflectStructToUI();

private slots:
    void checkCOM();
    void frequencySpinUpdate(int value);
    void frequencySliderUpdate(double arg1);
    void dutySpinUpdate(int value);
    void dutySliderUpdate(double value);
    void flagAll(bool b);
    void sendToMicroHardware();
    void sendToMicroSoftware();
    void HardwareTabGraph(int tabIndex);
    void graphChanged();
    void dutySliderHardwareUpdate(double value);
    void dutySpinUpdateHardware(int value);
    void graphChangedproxy(int index);
    void graphChangedproxy(double frequency);
    void setFlagHardware(bool b);

    void on_actionTutorial_triggered();
    void on_actionAbout_triggered();
    void on_actionSave_triggered();
    void on_actionLoad_triggered();

private:
    Ui::MainWindow *ui;
    tutorialDialog *TutDialog;
    About *aboutDialog;
    typedef struct
    {
        uint8_t inversion = 0;
        uint8_t pin;
        uint8_t PWM_DutyCycle;
        uint8_t FrequencyA;
        uint8_t FrequencyB;
        uint16_t Frequency;
    }PWMParamsSoftware;
    PWMParamsSoftware PPS;

    typedef struct
    {
        uint8_t inversion = 0;
        uint8_t pin;
        uint8_t PWM_Mode;
        uint8_t PWM_DutyCycle;
        uint8_t PWM_frequency_index;
        float uF;
    }PWMParamsHardware;
    PWMParamsHardware PPH;

    QString activePort;
    QString SerialPorts[20];
};
#endif // MAINWINDOW_H
