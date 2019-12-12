#include "dsp_signalreconstructweightings.h"
#include "ui_dsp_signalreconstructweightings.h"
#include <QSpinBox>


DSP_SignalReconstructWeightings::DSP_SignalReconstructWeightings(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_SignalReconstructWeightings)
{
    ui->setupUi(this);


    int num_scales = var_struct_ref.getNumAtrialReconScales();  //number of wavelet scales to be used
    for( int i=0; i<num_scales; i++ )
    {
        //Set a label per scale
        QLabel *label = new QLabel(this);
        label->setNum( i+1 );
        ui->verticalLayout_scales->insertWidget( i, label );

        //Alphas - set a double spinbox per scale and connect to lambda function changing variable in filter struct
        QDoubleSpinBox *spinBox = new QDoubleSpinBox(this);
        spinBox->setRange(0,1);
        spinBox->setSingleStep(0.05);
        spinBox->setDecimals(2);
        spinBox->setValue( var_struct_ref.atrial_recon_alphaParams(i) );
        ui->verticalLayout_alphas->insertWidget( i, spinBox );
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [=](double d){ var_struct_ref.atrial_recon_alphaParams(i) = d; });

        //HeightRatio - set a double spinbox per scale and connect to lambda function changing variable in filter struct
        QDoubleSpinBox *spinBoxHR = new QDoubleSpinBox(this);
        spinBoxHR->setRange(0,1);
        spinBoxHR->setSingleStep(0.05);
        spinBoxHR->setDecimals(2);
        spinBoxHR->setValue( var_struct_ref.atrial_recon_subLenHeightRatios(i));
        ui->verticalLayout_heightratios->insertWidget( i, spinBoxHR );
        connect(spinBoxHR, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [=](double d){ var_struct_ref.atrial_recon_subLenHeightRatios(i) = d; });
    }




}

DSP_SignalReconstructWeightings::~DSP_SignalReconstructWeightings()
{
    delete ui;
}
