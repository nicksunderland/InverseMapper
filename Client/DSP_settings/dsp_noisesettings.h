#ifndef DSP_NOISESETTINGS_H
#define DSP_NOISESETTINGS_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_NoiseSettings;
}

class DSP_NoiseSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_NoiseSettings(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_NoiseSettings();

private slots:
    void on_spinBox_bandstopFilterOrder_valueChanged(int filter_order);
    void on_doubleSpinBox_bandstopLowerFreq_valueChanged(double filter_lower);
    void on_doubleSpinBox_bandstopUpperFreq_valueChanged(double filter_upper);
    void on_doubleSpinBox_highpassFreq_valueChanged(double highPass);
    void on_doubleSpinBox_lowpassFreq_valueChanged(double lowPass);
    void on_spinBox_lowPassFilterOrder_valueChanged(int lowpass_order);
    void on_spinBox_highPassFilterOrder_valueChanged(int highpass_order);
    void on_checkBox_use_bandstop_toggled(bool checked);
    void on_checkBox_use_highpass_toggled(bool checked);
    void on_checkBox_use_lowpass_toggled(bool checked);

    void on_checkBox_det9_toggled(bool checked);
    void on_checkBox_det8_toggled(bool checked);
    void on_checkBox_det7_toggled(bool checked);
    void on_checkBox_det6_toggled(bool checked);
    void on_checkBox_det5_toggled(bool checked);
    void on_checkBox_det4_toggled(bool checked);
    void on_checkBox_det3_toggled(bool checked);
    void on_checkBox_det2_toggled(bool checked);
    void on_checkBox_det1_toggled(bool checked);
    void on_checkBox_approx_toggled(bool checked);
    void on_checkBox_use_DWTdriftremove_toggled(bool checked);

    void on_doubleSpinBox_approxThresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det1Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det2Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det3Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det4Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det5Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det6Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det7Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det8Thresh_valueChanged(double thresh);
    void on_doubleSpinBox_Det9Thresh_valueChanged(double thresh);

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_NoiseSettings *ui;
};

#endif // DSP_NOISESETTINGS_H
