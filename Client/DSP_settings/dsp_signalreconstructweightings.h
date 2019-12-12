#ifndef DSP_SIGNALRECONSTRUCTWEIGHTINGS_H
#define DSP_SIGNALRECONSTRUCTWEIGHTINGS_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_SignalReconstructWeightings;
}

class DSP_SignalReconstructWeightings : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_SignalReconstructWeightings(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_SignalReconstructWeightings();

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_SignalReconstructWeightings *ui;
};

#endif // DSP_SIGNALRECONSTRUCTWEIGHTINGS_H
