#ifndef DSP_ATRIALACTIVATIONFINDER_H
#define DSP_ATRIALACTIVATIONFINDER_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_AtrialActivationFinder;
}

class DSP_AtrialActivationFinder : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_AtrialActivationFinder(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_AtrialActivationFinder();

private slots:
    void on_doubleSpinBox_waveletScaleMin_valueChanged(double scale_min);
    void on_doubleSpinBox_waveletScaleRes_valueChanged(double scale_res);
    void on_spinBox_numScales_valueChanged(int num_scales);
    void on_spinBox_waveletOrder_valueChanged(int order);
    void on_doubleSpinBox_threshold_valueChanged(double threshold);
    void on_doubleSpinBox_lagWindowMsec_valueChanged(double lagWin);
    void on_doubleSpinBox_stepWindowMsec_valueChanged(double stepWin);
    void on_doubleSpinBox_minActiInterval_valueChanged(double minInterval);

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_AtrialActivationFinder *ui;
};

#endif // DSP_ATRIALACTIVATIONFINDER_H
