#ifndef SIGNALPROCESSINGDIALOG_H
#define SIGNALPROCESSINGDIALOG_H

#include <QDialog>
#include "processing.h"
#include "pickwindow.h"

namespace Ui {
class SignalProcessingDialog;
}

class SignalProcessingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignalProcessingDialog(Processing& processing_ref,
                                    std::vector<QSharedPointer<Mesh> > &meshList_in,
                                    QDialog *parent = nullptr);
    ~SignalProcessingDialog();

signals:
    void SPD_user_scaling_signal(float max_y, float min_y, bool use_user_scaling );

private slots:
    void on_toolButton_noiseFilterSettings_clicked();
    void on_toolButton_findQRSsettings_clicked();
    void on_toolButton_QRSsubtract_clicked();
    void on_toolButton_findCathActivations_clicked();
    void on_toolButton_cathSigReconstruct_clicked();
    void on_toolButton_findEndoActivations_clicked();
    void on_checkBox_useSignalForInverse_toggled(bool checked);
    void on_doubleSpinBox_sampleRate_valueChanged(double sample_rate_in);
    void on_spinBox_basketElectrode_valueChanged(int basket_electrode);
    void on_toolButton_inverse_clicked();
    void on_checkBox_autoscaling_toggled(bool checked);
    void on_doubleSpinBox_mVmin_valueChanged(double arg1);
    void on_doubleSpinBox_mVmax_valueChanged(double arg1);

private:
    Ui::SignalProcessingDialog *ui;
    Processing& processing;
    Pickwindow* EMG_raw;
    Pickwindow* EMG_filtered;
    Pickwindow* EMG_atrial;

    //Meshes list (pointers) - if using make sure to check if null first
    std::vector< QSharedPointer<Mesh> > meshList;

};

#endif // SIGNALPROCESSINGDIALOG_H
