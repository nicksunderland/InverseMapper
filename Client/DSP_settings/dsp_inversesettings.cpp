#include "dsp_inversesettings.h"
#include "ui_dsp_inversesettings.h"

DSP_InverseSettings::DSP_InverseSettings(Inverse_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_InverseSettings)
{
    ui->setupUi(this);

    ui->label_numAtrialVerts->setNum( var_struct_ref.num_atrial_nodes );
    ui->label_numCathElectrodes->setNum( var_struct_ref.getNumCathChannels() );
    int goodChans = (var_struct_ref.cath_channels_to_use.array() == 1).count();
    ui->label_numCathElectrodesToBeUsed->setNum( goodChans );
    ui->doubleSpinBox_sourceDownsampling->setValue( var_struct_ref.atrial_inverse_source_downsampling );
    this->updateNumSources( var_struct_ref.atrial_inverse_source_downsampling );

    if( vars.lambdaMethod == LambdaMethod::userDefined )
    {
        ui->radioButton_inverseRegMethod_USER->setChecked( true );
    }else if (vars.lambdaMethod == LambdaMethod::CRESO) {
        ui->radioButton_inverseRegMethod_CRESO->setChecked( true );
        ui->doubleSpinBox_lambda->setEnabled( false );
    }
    if( vars.method == SourceGen::alongNormals )
    {
        ui->radioButton_sourcePosMethod_normals->setChecked( true );
    }else if ( vars.method == SourceGen::fromCentre ) {
        ui->radioButton_sourcePosMethod_centre->setChecked( true );
    }

    ui->doubleSpinBox_cathSourceScale->setValue( -vars.source_scaling[catheter] );
    ui->doubleSpinBox_atrialSourceScale->setValue( vars.source_scaling[atrium ] );

}

DSP_InverseSettings::~DSP_InverseSettings()
{
    delete ui;
}

void DSP_InverseSettings::on_doubleSpinBox_sourceDownsampling_valueChanged(double downsampling)
{
    this->updateNumSources( downsampling );
}

void DSP_InverseSettings::updateNumSources(double downsampling)
{
    var_struct_ref.atrial_inverse_source_downsampling = downsampling;
    ui->label_numAtrialSources->setNum( int(downsampling * var_struct_ref.num_atrial_nodes) );
}

void DSP_InverseSettings::on_radioButton_inverseRegMethod_USER_toggled(bool checked)
{
    if( checked )
    {
        var_struct_ref.lambdaMethod = LambdaMethod::userDefined;
        ui->doubleSpinBox_lambda->setEnabled( true );
        ui->doubleSpinBox_lambda->setValue( double(var_struct_ref.lambda) );
    }
}

void DSP_InverseSettings::on_radioButton_inverseRegMethod_CRESO_toggled(bool checked)
{
    if( checked )
    {
        var_struct_ref.lambdaMethod = LambdaMethod::CRESO;
        ui->doubleSpinBox_lambda->setValue( 0.0 );
        ui->doubleSpinBox_lambda->setEnabled( false );
    }
}

void DSP_InverseSettings::on_radioButton_sourcePosMethod_normals_toggled(bool checked)
{
    if( checked)
    {
        var_struct_ref.method = SourceGen::alongNormals;
    }
}

void DSP_InverseSettings::on_radioButton_sourcePosMethod_centre_toggled(bool checked)
{
    if( checked)
    {
        var_struct_ref.method = SourceGen::fromCentre;
    }
}

void DSP_InverseSettings::on_doubleSpinBox_cathSourceScale_valueChanged(double cathScale)
{
    var_struct_ref.source_scaling[ catheter ] = -cathScale;
}

void DSP_InverseSettings::on_doubleSpinBox_atrialSourceScale_valueChanged(double atriumScale)
{
    var_struct_ref.source_scaling[ atrium ] = atriumScale;
}
