#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "geomwindow.h"
#include "processing.h"

#include <chrono>

using namespace std::chrono;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void time_frame_changed( int time );
    void mesh_data_max_min_signal( float mVmax, float mVmin, bool user_scaling );
    void pick_windows_changed( std::vector<Pickwindow*> &pick_win_list );

public slots:
    void setMeshList(std::vector< QSharedPointer<Mesh> >& meshList_in );
    void connect_new_pickwindow(Pickwindow *newPickWin);
    void updateGeomwinLabel();

private slots:
    //Mesh load buttons
    void on_toolButton_atrialGeom_clicked();
    void on_toolButton_atrialData_clicked();
    void on_toolButton_cathGeom_clicked();
    void on_toolButton_cathData_clicked();
    void on_pushButton_loadFiles_clicked();
    void on_comboBox_default_studies_currentIndexChanged(int index);

    //Movie controls
    void on_toolButton_play_clicked();
    void on_toolButton_pause_clicked();
    void on_toolButton_front_clicked();
    void on_toolButton_stepback_clicked();
    void on_toolButton_stepforward_clicked();
    void on_toolButton_end_clicked();
    void advanceTimer();
    void on_timeFramesPerSecSpinBox_valueChanged(int fps);
    void on_timeFrameSpinBox_valueChanged(int timeframe);
    void on_horizontalSlider_valueChanged(int value);

    //Run mode
    void on_radioButton_raw_basket_signal_mode_toggled( );
    void on_radioButton_atrial_signals_mode_toggled( );

    //Picks
    void on_pushButton_deleteLastPick_clicked();
    void on_checkBox_showAtrialElectrogram_toggled(bool checked);
    void on_checkBox_showNearElectrode_toggled(bool checked);

    //Window view controls
    void on_toolButton_win1_clicked();
    void on_toolButton_win2_clicked();
    void on_toolButton_win3_clicked();
    void on_doubleSpinBox_mVmax_valueChanged(double max);
    void on_doubleSpinBox_mVmin_valueChanged(double min);
    void on_checkBox_autoScaling_toggled(bool checked);

    //Signal processing
    void on_pushButton_signalProcessDialog_clicked();
    void on_toolButton_actiHistMapSettings_clicked();
    void on_toolButton_inverseSettings_clicked();
    void on_toolButton_phaseMapSettings_clicked();

private:
    Ui::MainWindow *ui;
    Processing processing;
    std::vector< QSharedPointer<Mesh> > meshList;     //Meshes list (pointers) - if using make sure to check if null first


    //Movie and timer
    QTimer timer;
    int current_time;
    int max_num_time_frames;
    int frames_per_sec;



    //Private initialising functions
    void makeConnections();
    void setupControls();
    void setupWindows();
    void extractDataInfo();
};

#endif // MAINWINDOW_H
