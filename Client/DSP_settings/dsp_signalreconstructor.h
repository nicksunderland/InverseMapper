#ifndef DSP_SIGNALRECONSTRUCTOR_H
#define DSP_SIGNALRECONSTRUCTOR_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_SignalReconstructor;
}

class DSP_SignalReconstructor : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_SignalReconstructor(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_SignalReconstructor();

private slots:
    void on_spinBox_waveletOrder_valueChanged(int order);

    void on_spinBox_numScales_valueChanged(int scales);

    void on_doubleSpinBox_waveletScaleRes_valueChanged(double resolution);

    void on_doubleSpinBox_waveletScaleMin_valueChanged(double min);

    void on_doubleSpinBox_preQRSmsec_valueChanged(double preActi);

    void on_doubleSpinBox_postQRSmsec_valueChanged(double postActi);

    void on_pushButton_subtractWeightingsDialog_clicked();

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_SignalReconstructor *ui;
};

#endif // DSP_SIGNALRECONSTRUCTOR_H
