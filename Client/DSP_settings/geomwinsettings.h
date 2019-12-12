#ifndef GEOMWINSETTINGS_H
#define GEOMWINSETTINGS_H

#include <QDialog>
#include "definesandstructs.h"

enum MeshView{
    wire,filled,points
};

struct GeomWinSettingsStruct{

    MeshView mesh_view_atrium;
    DataType data_view_atrium;
    MeshView mesh_view_cath;
    DataType data_view_cath;



    GeomWinSettingsStruct():
        mesh_view_atrium( MeshView::filled ),
        data_view_atrium( DataType::processed ),
        mesh_view_cath( MeshView::points ),
        data_view_cath( DataType::raw )
    {

    }

    void setDefaults( int winIndex, Mode run_mode )
    {
        if      ( winIndex==0 && run_mode==Mode::catheter_input )
        {
            mesh_view_cath   = MeshView::points;
            data_view_cath   = DataType::raw;
            mesh_view_atrium = MeshView::filled;
            data_view_atrium = DataType::acti_history;

        }else if( winIndex==1 && run_mode==Mode::catheter_input )
        {
            mesh_view_cath   = MeshView::points;
            data_view_cath   = DataType::raw;
            mesh_view_atrium = MeshView::filled;
            data_view_atrium = DataType::processed;

        }else if( winIndex==2 && run_mode==Mode::catheter_input )
        {
            mesh_view_cath   = MeshView::points;
            data_view_cath   = DataType::raw;
            mesh_view_atrium = MeshView::wire;
            data_view_atrium = DataType::no_data;

        }else if( winIndex==0 && run_mode==Mode::atrial_input )
        {
            mesh_view_cath   = MeshView::points;
            data_view_cath   = DataType::processed;
            mesh_view_atrium = MeshView::filled;
            data_view_atrium = DataType::raw;

        }else if( winIndex==1 && run_mode==Mode::atrial_input )
        {
            mesh_view_cath   = MeshView::points;
            data_view_cath   = DataType::processed;
            mesh_view_atrium = MeshView::filled;
            data_view_atrium = DataType::processed;

        }else if( winIndex==2 && run_mode==Mode::atrial_input )
        {
            mesh_view_cath   = MeshView::points;
            data_view_cath   = DataType::processed;
            mesh_view_atrium = MeshView::wire;
            data_view_atrium = DataType::no_data;

        }

    }

};

namespace Ui {
class GeomwinSettings;
}

class GeomwinSettings : public QDialog
{
    Q_OBJECT

public:
    explicit GeomwinSettings(GeomWinSettingsStruct &struct_in, QDialog *parent = nullptr);
    ~GeomwinSettings();

signals:
    void geomwin_settings_changed_signal();

private slots:
    void on_radioButton_pointAtrium_toggled(bool checked);
    void on_radioButton_filledAtrium_toggled(bool checked);
    void on_radioButton_wireAtrium_toggled(bool checked);
    void on_radioButton_pointCath_toggled(bool checked);
    void on_radioButton_filledCath_toggled(bool checked);
    void on_radioButton_wireCath_toggled(bool checked);
    void on_radioButton_rawDataCath_toggled(bool checked);
    void on_radioButton_procDataCath_toggled(bool checked);
    void on_radioButton_rawDataAtrium_toggled(bool checked);
    void on_radioButton_procDataAtrium_toggled(bool checked);
    void on_radioButton_actiHistDataAtrium_toggled(bool checked);
    void on_radioButton_AtrialDataOff_toggled(bool checked);
    void on_radioButton_cathDataOff_toggled(bool checked);

    void on_radioButton_phaseAtrium_toggled(bool checked);

private:
    Ui::GeomwinSettings *ui;
    GeomWinSettingsStruct& settings;
};

#endif // GEOMWINSETTINGS_H
