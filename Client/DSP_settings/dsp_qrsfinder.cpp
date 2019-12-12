#include "dsp_qrsfinder.h"
#include "ui_dsp_qrsfinder.h"

DSP_QRSfinder::DSP_QRSfinder(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_QRSfinder)
{
    ui->setupUi(this);

    ui->spinBox_QRSchannel->setValue( var_struct_ref.QRS_channel+1 );
    ui->doubleSpinBox_qrsFinderThreshold->setValue( var_struct_ref.QRS_threshold );
    if( var_struct_ref.QRS_direction == true )
    {
        ui->radioButton_QRSpositive->setChecked( true );
    }else {
        ui->radioButton_QRSnegative->setChecked( true );
    }

}

DSP_QRSfinder::~DSP_QRSfinder()
{
    delete ui;
}

void DSP_QRSfinder::on_doubleSpinBox_qrsFinderThreshold_valueChanged(double threshold)
{
    var_struct_ref.QRS_threshold = threshold;
}

void DSP_QRSfinder::on_spinBox_QRSchannel_valueChanged(int channel)
{
    var_struct_ref.QRS_channel = channel-1;
}

void DSP_QRSfinder::on_radioButton_QRSpositive_toggled(bool checked)
{
    if( checked )
    {
        var_struct_ref.QRS_direction = true;
    }
}

void DSP_QRSfinder::on_radioButton_QRSnegative_toggled(bool checked)
{
    if( checked )
    {
        var_struct_ref.QRS_direction = false;
    }
}
