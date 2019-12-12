#ifndef DSP_INVERSESETTINGS_H
#define DSP_INVERSESETTINGS_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_InverseSettings;
}

class DSP_InverseSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_InverseSettings(Inverse_vars &vars, QWidget *parent = nullptr);
    ~DSP_InverseSettings();

private slots:
    void on_doubleSpinBox_sourceDownsampling_valueChanged(double downsampling);
    void on_radioButton_inverseRegMethod_USER_toggled(bool checked);
    void on_radioButton_inverseRegMethod_CRESO_toggled(bool checked);
    void on_radioButton_sourcePosMethod_normals_toggled(bool checked);
    void on_radioButton_sourcePosMethod_centre_toggled(bool checked);

    void on_doubleSpinBox_cathSourceScale_valueChanged(double cathScale);

    void on_doubleSpinBox_atrialSourceScale_valueChanged(double atriumScale);

private:
    Inverse_vars &var_struct_ref;
    Ui::DSP_InverseSettings *ui;
    void updateNumSources(double downsampling);
};

#endif // DSP_INVERSESETTINGS_H
