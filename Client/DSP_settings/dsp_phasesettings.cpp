#include "dsp_phasesettings.h"
#include "ui_dsp_phasesettings.h"

DSP_PhaseSettings::DSP_PhaseSettings(Filter_vars &vars, QWidget *parent) :
    QDialog(parent),
    var_struct_ref(vars),
    ui(new Ui::DSP_PhaseSettings)
{
    ui->setupUi(this);

    ui->doubleSpinBox_recompPeriod->setValue( vars.getSampleRate() * vars.phase_base_period_secs );
}

DSP_PhaseSettings::~DSP_PhaseSettings()
{
    delete ui;
}

void DSP_PhaseSettings::on_doubleSpinBox_recompPeriod_valueChanged(double period)
{
    var_struct_ref.phase_base_period_secs = period / 1000.0;
}
