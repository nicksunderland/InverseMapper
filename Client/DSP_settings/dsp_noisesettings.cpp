#include "dsp_noisesettings.h"
#include "ui_dsp_noisesettings.h"

DSP_NoiseSettings::DSP_NoiseSettings(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_NoiseSettings)
{
    ui->setupUi(this);

    //Set the spin boxes to the current values
    ui->spinBox_bandstopFilterOrder->setValue( var_struct_ref.bandstop_order );
    ui->doubleSpinBox_bandstopLowerFreq->setValue( var_struct_ref.bandstop_lower );
    ui->doubleSpinBox_bandstopUpperFreq->setValue( var_struct_ref.bandstop_upper );
    ui->doubleSpinBox_lowpassFreq->setValue( var_struct_ref.lowpass_cutoff );
    ui->doubleSpinBox_highpassFreq->setValue(  var_struct_ref.highpass_cutoff );
    ui->checkBox_use_bandstop->setChecked( var_struct_ref.bool_use_bandstop );
    ui->checkBox_use_highpass->setChecked( var_struct_ref.bool_use_highpass );
    ui->checkBox_use_lowpass->setChecked( var_struct_ref.bool_use_lowpass );

    ui->checkBox_use_DWTdriftremove->setChecked( var_struct_ref.bool_use_DWTdrift_remove );
    ui->checkBox_approx->setChecked( var_struct_ref.DWTdrift_levels_to_keep[0] );
    ui->checkBox_det1->setChecked( var_struct_ref.DWTdrift_levels_to_keep[1] );
    ui->checkBox_det2->setChecked( var_struct_ref.DWTdrift_levels_to_keep[2] );
    ui->checkBox_det3->setChecked( var_struct_ref.DWTdrift_levels_to_keep[3] );
    ui->checkBox_det4->setChecked( var_struct_ref.DWTdrift_levels_to_keep[4] );
    ui->checkBox_det5->setChecked( var_struct_ref.DWTdrift_levels_to_keep[5] );
    ui->checkBox_det6->setChecked( var_struct_ref.DWTdrift_levels_to_keep[6] );
    ui->checkBox_det7->setChecked( var_struct_ref.DWTdrift_levels_to_keep[7] );
    ui->checkBox_det8->setChecked( var_struct_ref.DWTdrift_levels_to_keep[8] );
    ui->checkBox_det9->setChecked( var_struct_ref.DWTdrift_levels_to_keep[9] );
    ui->doubleSpinBox_approxThresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(0) );
    ui->doubleSpinBox_Det1Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(1) );
    ui->doubleSpinBox_Det2Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(2) );
    ui->doubleSpinBox_Det3Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(3) );
    ui->doubleSpinBox_Det4Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(4) );
    ui->doubleSpinBox_Det5Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(5) );
    ui->doubleSpinBox_Det6Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(6) );
    ui->doubleSpinBox_Det7Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(7) );
    ui->doubleSpinBox_Det8Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(8) );
    ui->doubleSpinBox_Det9Thresh->setValue( var_struct_ref.DWTdrift_levels_std_weights(9) );


}

DSP_NoiseSettings::~DSP_NoiseSettings()
{
    delete ui;
}

void DSP_NoiseSettings::on_spinBox_bandstopFilterOrder_valueChanged(int filter_order)
{
    var_struct_ref.bandstop_order = filter_order;
}

void DSP_NoiseSettings::on_doubleSpinBox_bandstopLowerFreq_valueChanged(double filter_lower)
{
    if( filter_lower >= ui->doubleSpinBox_bandstopUpperFreq->value() )
    {
        ui->doubleSpinBox_bandstopLowerFreq->setValue( var_struct_ref.bandstop_lower );
    }else {
        var_struct_ref.bandstop_lower = filter_lower;
    }
}

void DSP_NoiseSettings::on_doubleSpinBox_bandstopUpperFreq_valueChanged(double filter_upper)
{
    if( filter_upper <= ui->doubleSpinBox_bandstopLowerFreq->value() )
    {
        ui->doubleSpinBox_bandstopUpperFreq->setValue( var_struct_ref.bandstop_upper );
    }else {
        var_struct_ref.bandstop_upper = filter_upper;
    }
}

void DSP_NoiseSettings::on_doubleSpinBox_highpassFreq_valueChanged(double highPass)
{
    if( highPass >= ui->doubleSpinBox_bandstopLowerFreq->value() )
    {
        ui->doubleSpinBox_highpassFreq->setValue( var_struct_ref.highpass_cutoff );
    }else {
        var_struct_ref.highpass_cutoff = highPass;
    }
}

void DSP_NoiseSettings::on_doubleSpinBox_lowpassFreq_valueChanged(double lowPass)
{
    if( lowPass <= ui->doubleSpinBox_bandstopUpperFreq->value() )
    {
        ui->doubleSpinBox_lowpassFreq->setValue( var_struct_ref.lowpass_cutoff );
    }else {
        var_struct_ref.lowpass_cutoff = lowPass;
    }
}

void DSP_NoiseSettings::on_spinBox_lowPassFilterOrder_valueChanged(int lowpass_order)
{
    var_struct_ref.lowpass_order = lowpass_order;
}

void DSP_NoiseSettings::on_spinBox_highPassFilterOrder_valueChanged(int highpass_order)
{
    var_struct_ref.highpass_order = highpass_order;
}

void DSP_NoiseSettings::on_checkBox_use_bandstop_toggled(bool checked)
{
    var_struct_ref.bool_use_bandstop = checked;
}

void DSP_NoiseSettings::on_checkBox_use_highpass_toggled(bool checked)
{
    var_struct_ref.bool_use_highpass = checked;
}

void DSP_NoiseSettings::on_checkBox_use_lowpass_toggled(bool checked)
{
    var_struct_ref.bool_use_lowpass = checked;
}

void DSP_NoiseSettings::on_checkBox_det9_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[9] = checked;
}

void DSP_NoiseSettings::on_checkBox_det8_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[8] = checked;
}

void DSP_NoiseSettings::on_checkBox_det7_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[7] = checked;
}

void DSP_NoiseSettings::on_checkBox_det6_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[6] = checked;
}

void DSP_NoiseSettings::on_checkBox_det5_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[5] = checked;
}

void DSP_NoiseSettings::on_checkBox_det4_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[4] = checked;
}

void DSP_NoiseSettings::on_checkBox_det3_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[3] = checked;
}

void DSP_NoiseSettings::on_checkBox_det2_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[2] = checked;
}

void DSP_NoiseSettings::on_checkBox_det1_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[1] = checked;
}

void DSP_NoiseSettings::on_checkBox_approx_toggled(bool checked)
{
    var_struct_ref.DWTdrift_levels_to_keep[0] = checked;
}

void DSP_NoiseSettings::on_checkBox_use_DWTdriftremove_toggled(bool checked)
{
    var_struct_ref.bool_use_DWTdrift_remove = checked;
}

void DSP_NoiseSettings::on_doubleSpinBox_approxThresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(0) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det1Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(1) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det2Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(2) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det3Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(3) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det4Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(4) = thresh;
}


void DSP_NoiseSettings::on_doubleSpinBox_Det5Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(5) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det6Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(6) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det7Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(7) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det8Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(8) = thresh;
}

void DSP_NoiseSettings::on_doubleSpinBox_Det9Thresh_valueChanged(double thresh)
{
    var_struct_ref.DWTdrift_levels_std_weights(9) = thresh;
}
