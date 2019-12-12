#ifndef DSP_PHASESETTINGS_H
#define DSP_PHASESETTINGS_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_PhaseSettings;
}

class DSP_PhaseSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_PhaseSettings(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_PhaseSettings();

private slots:
    void on_doubleSpinBox_recompPeriod_valueChanged(double period);

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_PhaseSettings *ui;
};

#endif // DSP_PHASESETTINGS_H
