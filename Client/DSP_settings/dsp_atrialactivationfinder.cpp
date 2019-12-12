#include "dsp_atrialactivationfinder.h"
#include "ui_dsp_atrialactivationfinder.h"

DSP_AtrialActivationFinder::DSP_AtrialActivationFinder(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_AtrialActivationFinder)
{
    ui->setupUi(this);

    ui->spinBox_waveletOrder->setValue( var_struct_ref.atrial_search_wavelet_order );
    ui->spinBox_numScales->setValue( var_struct_ref.atrial_search_num_scales );
    ui->doubleSpinBox_waveletScaleMin->setValue( var_struct_ref.atrial_search_scale_min );
    ui->doubleSpinBox_waveletScaleRes->setValue( var_struct_ref.atrial_search_scale_res );
    ui->doubleSpinBox_threshold->setValue( var_struct_ref.atrial_search_threshold );
    ui->doubleSpinBox_lagWindowMsec->setValue( var_struct_ref.atrial_search_lag_win_length_secs*1000 );
    ui->doubleSpinBox_stepWindowMsec->setValue( var_struct_ref.atrial_search_step_win_length_secs*1000 );
    ui->doubleSpinBox_minActiInterval->setValue( var_struct_ref.atrial_search_min_activation_interval_secs*1000 );
}

DSP_AtrialActivationFinder::~DSP_AtrialActivationFinder()
{
    delete ui;
}

void DSP_AtrialActivationFinder::on_doubleSpinBox_waveletScaleMin_valueChanged(double scale_min)
{
    var_struct_ref.atrial_search_scale_min = scale_min;
}

void DSP_AtrialActivationFinder::on_doubleSpinBox_waveletScaleRes_valueChanged(double scale_res)
{
    var_struct_ref.atrial_search_scale_res = scale_res;
}

void DSP_AtrialActivationFinder::on_spinBox_numScales_valueChanged(int num_scales)
{
    var_struct_ref.atrial_search_num_scales = num_scales;
}

void DSP_AtrialActivationFinder::on_spinBox_waveletOrder_valueChanged(int order)
{
    var_struct_ref.atrial_recon_wavelet_order = order;
}

void DSP_AtrialActivationFinder::on_doubleSpinBox_threshold_valueChanged(double threshold)
{
    var_struct_ref.atrial_search_threshold = threshold;
}

void DSP_AtrialActivationFinder::on_doubleSpinBox_lagWindowMsec_valueChanged(double lagWin)
{
    var_struct_ref.atrial_search_lag_win_length_secs = ( lagWin / 1000.0 );
}

void DSP_AtrialActivationFinder::on_doubleSpinBox_stepWindowMsec_valueChanged(double stepWin)
{
    var_struct_ref.atrial_search_step_win_length_secs = ( stepWin / 1000.0 );
}

void DSP_AtrialActivationFinder::on_doubleSpinBox_minActiInterval_valueChanged(double minInterval)
{
    var_struct_ref.atrial_search_min_activation_interval_secs = ( minInterval / 1000 );
}
