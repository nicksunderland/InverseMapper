#include "dsp_qrssubtraction.h"
#include "ui_dsp_qrssubtraction.h"
#include "dsp_qrssubtractionweightings.h"

DSP_QRSsubtraction::DSP_QRSsubtraction(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_QRSsubtraction)
{
    ui->setupUi(this);

    ui->spinBox_waveletOrder->setValue( var_struct_ref.QRSsub_wavelet_order );
    ui->spinBox_numScales->setValue( var_struct_ref.getNumQRSSubScales() );
    ui->doubleSpinBox_waveletScaleMin->setValue( var_struct_ref.QRSsub_scale_min );
    ui->doubleSpinBox_waveletScaleRes->setValue( var_struct_ref.QRSsub_scale_res );
    ui->doubleSpinBox_preQRSmsec->setValue( var_struct_ref.QRSsub_preQRSwin * 1000.0 );
    ui->doubleSpinBox_postQRSmsec->setValue( var_struct_ref.QRSsub_postQRSwin * 1000.0 );
}

DSP_QRSsubtraction::~DSP_QRSsubtraction()
{
    delete ui;
}

void DSP_QRSsubtraction::on_spinBox_waveletOrder_valueChanged(int order)
{
    var_struct_ref.QRSsub_wavelet_order = order;
}

void DSP_QRSsubtraction::on_doubleSpinBox_waveletScaleMin_valueChanged(double scale_min)
{
    var_struct_ref.QRSsub_scale_min = scale_min;
}

void DSP_QRSsubtraction::on_doubleSpinBox_waveletScaleRes_valueChanged(double scale_res)
{
    var_struct_ref.QRSsub_scale_res = scale_res;
}

void DSP_QRSsubtraction::on_spinBox_numScales_valueChanged(int num_scales)
{
    var_struct_ref.changeQRSSub_NumScales( num_scales );
}

void DSP_QRSsubtraction::on_doubleSpinBox_preQRSmsec_valueChanged(double preQRS)
{
    var_struct_ref.QRSsub_preQRSwin = preQRS / 1000.0;
}

void DSP_QRSsubtraction::on_doubleSpinBox_postQRSmsec_valueChanged(double postQRS)
{
    var_struct_ref.QRSsub_postQRSwin = postQRS / 1000.0;
}

void DSP_QRSsubtraction::on_pushButton_subtractWeightingsDialog_clicked()
{
    DSP_QRSSubtractionWeightings *dialog = new DSP_QRSSubtractionWeightings(var_struct_ref, this);
    dialog->exec(); //modal window
}
