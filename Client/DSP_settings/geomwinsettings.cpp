#include "geomwinsettings.h"
#include "ui_geomwinsettings.h"

GeomwinSettings::GeomwinSettings(GeomWinSettingsStruct& struct_in, QDialog *parent) :
    QDialog (parent),
    ui(new Ui::GeomwinSettings),
    settings( struct_in )
{
    ui->setupUi(this);


//Cath mesh
    if( settings.mesh_view_cath==MeshView::wire )
    {
        ui->radioButton_wireCath->setChecked( true );
    }
    if( settings.mesh_view_cath==MeshView::filled )
    {
        ui->radioButton_filledCath->setChecked( true );
    }
    if( settings.mesh_view_cath==MeshView::points )
    {
        ui->radioButton_pointCath->setChecked( true );
    }

//Atrial mesh
    if( settings.mesh_view_atrium==MeshView::wire )
    {
        ui->radioButton_wireAtrium->setChecked( true );
    }
    if( settings.mesh_view_atrium==MeshView::filled )
    {
        ui->radioButton_filledAtrium->setChecked( true );
    }
    if( settings.mesh_view_atrium==MeshView::points )
    {
        ui->radioButton_pointAtrium->setChecked( true );
    }

//Cath data
    if( settings.data_view_cath==DataType::raw )
    {
        ui->radioButton_rawDataCath->setChecked( true );
    }
    if( settings.data_view_cath==DataType::processed )
    {
        ui->radioButton_procDataCath->setChecked( true );
    }
    if( settings.data_view_cath==DataType::no_data )
    {
        ui->radioButton_cathDataOff->setChecked( true );
    }

//Atrial data
    if( settings.data_view_atrium==DataType::raw )
    {
        ui->radioButton_rawDataAtrium->setChecked( true );
    }
    if( settings.data_view_atrium==DataType::processed )
    {
        ui->radioButton_procDataAtrium->setChecked( true );
    }
    if( settings.data_view_atrium==DataType::acti_history )
    {
        ui->radioButton_actiHistDataAtrium->setChecked( true );
    }
    if( settings.data_view_atrium==DataType::phase )
    {
        ui->radioButton_phaseAtrium->setChecked( true );
    }
    if( settings.data_view_atrium==DataType::no_data )
    {
        ui->radioButton_AtrialDataOff->setChecked( true );
    }

}

GeomwinSettings::~GeomwinSettings()
{
    delete ui;
}

void GeomwinSettings::on_radioButton_pointAtrium_toggled(bool checked)
{
    if( checked )
    {
        settings.mesh_view_atrium = MeshView::points;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_filledAtrium_toggled(bool checked)
{
    if( checked )
    {
        settings.mesh_view_atrium = MeshView::filled;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_wireAtrium_toggled(bool checked)
{
    if( checked )
    {
        settings.mesh_view_atrium = MeshView::wire;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_pointCath_toggled(bool checked)
{
    if( checked )
    {
        settings.mesh_view_cath = MeshView::points;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_filledCath_toggled(bool checked)
{
    if( checked )
    {
        settings.mesh_view_cath = MeshView::filled;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_wireCath_toggled(bool checked)
{
    if( checked )
    {
        settings.mesh_view_cath = MeshView::wire;
    }
    emit geomwin_settings_changed_signal();

}

void GeomwinSettings::on_radioButton_rawDataCath_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_cath = DataType::raw;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_procDataCath_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_cath = DataType::processed;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_rawDataAtrium_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_atrium = DataType::raw;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_procDataAtrium_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_atrium = DataType::processed;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_actiHistDataAtrium_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_atrium = DataType::acti_history;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_AtrialDataOff_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_atrium = DataType::no_data;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_cathDataOff_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_cath = DataType::no_data;
    }
    emit geomwin_settings_changed_signal();
}

void GeomwinSettings::on_radioButton_phaseAtrium_toggled(bool checked)
{
    if( checked )
    {
        settings.data_view_atrium = DataType::phase;
    }
    emit geomwin_settings_changed_signal();
}
