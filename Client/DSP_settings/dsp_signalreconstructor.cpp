#include "dsp_signalreconstructor.h"
#include "ui_dsp_signalreconstructor.h"
#include "dsp_signalreconstructweightings.h"

DSP_SignalReconstructor::DSP_SignalReconstructor(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_SignalReconstructor)
{
    ui->setupUi(this);

    ui->spinBox_waveletOrder->setValue( var_struct_ref.atrial_recon_wavelet_order );
    ui->spinBox_numScales->setValue( var_struct_ref.getNumAtrialReconScales() );
    ui->doubleSpinBox_waveletScaleMin->setValue( var_struct_ref.atrial_recon_scale_min );
    ui->doubleSpinBox_waveletScaleRes->setValue( var_struct_ref.atrial_recon_scale_res );
    ui->doubleSpinBox_preQRSmsec->setValue( var_struct_ref.atrial_recon_pre_win_secs * 1000.0 );
    ui->doubleSpinBox_postQRSmsec->setValue( var_struct_ref.atrial_recon_post_win_secs * 1000.0 );
}

DSP_SignalReconstructor::~DSP_SignalReconstructor()
{
    delete ui;
}

void DSP_SignalReconstructor::on_spinBox_waveletOrder_valueChanged(int order)
{
    var_struct_ref.atrial_recon_wavelet_order = order;
}

void DSP_SignalReconstructor::on_spinBox_numScales_valueChanged(int scales)
{
    var_struct_ref.setNumAtrialReconScales( scales );
}

void DSP_SignalReconstructor::on_doubleSpinBox_waveletScaleRes_valueChanged(double resolution)
{
    var_struct_ref.atrial_recon_scale_res = resolution;
}

void DSP_SignalReconstructor::on_doubleSpinBox_waveletScaleMin_valueChanged(double min)
{
    var_struct_ref.atrial_recon_scale_min = min;
}

void DSP_SignalReconstructor::on_doubleSpinBox_preQRSmsec_valueChanged(double preActi)
{
    var_struct_ref.atrial_recon_pre_win_secs =  preActi / 1000.0;
}

void DSP_SignalReconstructor::on_doubleSpinBox_postQRSmsec_valueChanged(double postActi)
{
    var_struct_ref.atrial_recon_post_win_secs =  postActi / 1000.0;
}

void DSP_SignalReconstructor::on_pushButton_subtractWeightingsDialog_clicked()
{
    DSP_SignalReconstructWeightings *dialog = new DSP_SignalReconstructWeightings(var_struct_ref, this);
    dialog->exec(); //modal window
}
