#ifndef DSP_ACTIVATIONHISTORYSETTINGS_H
#define DSP_ACTIVATIONHISTORYSETTINGS_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_ActivationHistorySettings;
}

class DSP_ActivationHistorySettings : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_ActivationHistorySettings(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_ActivationHistorySettings();

private slots:
    void on_doubleSpinBox_lagWindowMsec_valueChanged(double winLen);
    void on_doubleSpinBox_peakValue_valueChanged(double peakVal);

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_ActivationHistorySettings *ui;
};

#endif // DSP_ACTIVATIONHISTORYSETTINGS_H
