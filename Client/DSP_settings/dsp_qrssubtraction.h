#ifndef DSP_QRSSUBTRACTION_H
#define DSP_QRSSUBTRACTION_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_QRSsubtraction;
}

class DSP_QRSsubtraction : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_QRSsubtraction(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_QRSsubtraction();

private slots:
    void on_spinBox_waveletOrder_valueChanged(int order);
    void on_doubleSpinBox_waveletScaleMin_valueChanged(double scale_min);
    void on_doubleSpinBox_waveletScaleRes_valueChanged(double scale_res);
    void on_spinBox_numScales_valueChanged(int num_scales);
    void on_doubleSpinBox_preQRSmsec_valueChanged(double preQRS);
    void on_doubleSpinBox_postQRSmsec_valueChanged(double postQRS);
    void on_pushButton_subtractWeightingsDialog_clicked();

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_QRSsubtraction *ui;
};

#endif // DSP_QRSSUBTRACTION_H
