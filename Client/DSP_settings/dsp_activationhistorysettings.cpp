#include "dsp_activationhistorysettings.h"
#include "ui_dsp_activationhistorysettings.h"

DSP_ActivationHistorySettings::DSP_ActivationHistorySettings(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_ActivationHistorySettings)
{
    ui->setupUi(this);

    ui->doubleSpinBox_lagWindowMsec->setValue( var_struct_ref.activation_history_win_secs * 1000.0 );
    ui->doubleSpinBox_peakValue    ->setValue( var_struct_ref.activation_history_max_val );
}

DSP_ActivationHistorySettings::~DSP_ActivationHistorySettings()
{
    delete ui;
}

void DSP_ActivationHistorySettings::on_doubleSpinBox_lagWindowMsec_valueChanged(double winLen)
{
    var_struct_ref.activation_history_win_secs = winLen / 1000.0;
}

void DSP_ActivationHistorySettings::on_doubleSpinBox_peakValue_valueChanged(double peakVal)
{
    var_struct_ref.activation_history_win_secs = peakVal;
}
