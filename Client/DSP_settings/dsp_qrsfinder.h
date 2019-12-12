#ifndef DSP_QRSFINDER_H
#define DSP_QRSFINDER_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_QRSfinder;
}

class DSP_QRSfinder : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_QRSfinder(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_QRSfinder();

private slots:
    void on_doubleSpinBox_qrsFinderThreshold_valueChanged(double threshold);
    void on_spinBox_QRSchannel_valueChanged(int channel);

    void on_radioButton_QRSpositive_toggled(bool checked);

    void on_radioButton_QRSnegative_toggled(bool checked);

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_QRSfinder *ui;
};

#endif // DSP_QRSFINDER_H
