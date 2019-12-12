#include "dsp_qrssubtractionweightings.h"
#include "ui_dsp_qrssubtractionweightings.h"
#include <QSpinBox>
#include <iostream>

DSP_QRSSubtractionWeightings::DSP_QRSSubtractionWeightings(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_QRSSubtractionWeightings)
{
    ui->setupUi(this);

    int num_scales = var_struct_ref.getNumQRSSubScales();  //number of wavelet scales to be used
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
        spinBox->setValue( var_struct_ref.QRSsub_Tuk_alphaParams(i) );
        ui->verticalLayout_alphas->insertWidget( i, spinBox );
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [=](double d){ var_struct_ref.QRSsub_Tuk_alphaParams(i) = d; });
        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DSP_QRSSubtractionWeightings::spinBoxChanged_printNewValue );

        //Baselength - set a double spinbox per scale and connect to lambda function changing variable in filter struct
        QDoubleSpinBox *spinBoxBL = new QDoubleSpinBox(this);
        spinBoxBL->setRange(0,1000);
        spinBoxBL->setSingleStep(1);
        spinBoxBL->setDecimals(0);
        spinBoxBL->setValue( double(var_struct_ref.QRSsub_Tuk_subLengths(i)) );
        ui->verticalLayout_baselengths->insertWidget( i, spinBoxBL );
        connect(spinBoxBL, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [=](double d){ var_struct_ref.QRSsub_Tuk_subLengths(i) = int( (d/1000.0)*var_struct_ref.getSampleRate() ); });
        connect(spinBoxBL, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DSP_QRSSubtractionWeightings::spinBoxChanged_printNewValue );

        //HeightRatio - set a double spinbox per scale and connect to lambda function changing variable in filter struct
        QDoubleSpinBox *spinBoxHR = new QDoubleSpinBox(this);
        spinBoxHR->setRange(0,1);
        spinBoxHR->setSingleStep(0.05);
        spinBoxHR->setDecimals(2);
        spinBoxHR->setValue( var_struct_ref.QRSsub_Tuk_subLenHeightRatios(i));
        ui->verticalLayout_heightratios->insertWidget( i, spinBoxHR );
        connect(spinBoxHR, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [=](double d){ var_struct_ref.QRSsub_Tuk_subLenHeightRatios(i) = d; });
        connect(spinBoxHR, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DSP_QRSSubtractionWeightings::spinBoxChanged_printNewValue );
    }


}

DSP_QRSSubtractionWeightings::~DSP_QRSSubtractionWeightings()
{
    delete ui;
}

//Just for testing
void DSP_QRSSubtractionWeightings::spinBoxChanged_printNewValue( double index )
{
    std::cout << "Box index: " << index << std::endl;
    std::cout << "Struct alphas: " << var_struct_ref.QRSsub_Tuk_alphaParams.transpose() << std::endl;
    std::cout << "Struct BLs   : " << var_struct_ref.QRSsub_Tuk_subLengths.transpose() << std::endl;
    std::cout << "Struct HRs   : " << var_struct_ref.QRSsub_Tuk_subLenHeightRatios.transpose() << std::endl;

}
