#ifndef DSP_QRSSUBTRACTIONWEIGHTINGS_H
#define DSP_QRSSUBTRACTIONWEIGHTINGS_H

#include <QDialog>
#include "definesandstructs.h"

namespace Ui {
class DSP_QRSSubtractionWeightings;
}

class DSP_QRSSubtractionWeightings : public QDialog
{
    Q_OBJECT

public:
    explicit DSP_QRSSubtractionWeightings(Filter_vars &vars, QWidget *parent = nullptr);
    ~DSP_QRSSubtractionWeightings();

public slots:
    void spinBoxChanged_printNewValue(double index);

private:
    Filter_vars &var_struct_ref;
    Ui::DSP_QRSSubtractionWeightings *ui;

};

#endif // DSP_QRSSUBTRACTIONWEIGHTINGS_H
