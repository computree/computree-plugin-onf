/****************************************************************************
 Copyright (C) 2010-2012 the Office National des Forêts (ONF), France
                         All rights reserved.

 Contact : alexandre.piboule@onf.fr

 Developers : Alexandre PIBOULE (ONF)

 This file is part of PluginONF library.

 PluginONF is free library: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 PluginONF is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with PluginONF.  If not, see <http://www.gnu.org/licenses/lgpl.html>.
*****************************************************************************/

#ifndef ONF_ACTIONSEGMENTCROWNSOPTIONS_H
#define ONF_ACTIONSEGMENTCROWNSOPTIONS_H

#include "ct_view/actions/abstract/ct_gabstractactionoptions.h"

class ONF_ActionSegmentCrowns;

namespace Ui {
class ONF_ActionSegmentCrownsOptions;
}

class ONF_ActionSegmentCrownsOptions : public CT_GAbstractActionOptions
{
    Q_OBJECT

public:

    enum Mode {
        CHOOSECLUSTER,
        DRAWLIMITS,
        FILLAREA,
        FREEMOVE,
        CHANGECENTERCELL
    };

    enum DrawingGrid {
        DENSITY,
        HEIGHT,
        CLUSTERS
    };

    explicit ONF_ActionSegmentCrownsOptions(const ONF_ActionSegmentCrowns *action, float initZvalue);
    ~ONF_ActionSegmentCrownsOptions();

    float getHeight();
    int getActiveCluster();
    bool getShowClustersOnly();
    double getHSingleStep();
    int getPencilSize();
    bool redForSelection();
    bool isFillModeFull();

    Mode getMode();
    DrawingGrid getDrawingGrid();

private:
    Ui::ONF_ActionSegmentCrownsOptions *ui;

    Mode                _mode;
    DrawingGrid         _drawingGrid;

public slots:
    void setHeight(float h);
    void setActiveCluster(int cluster);
    void setClusterNumber(int nb);
    void setMode(Mode mode);
    void setDrawingGrid(DrawingGrid drawingGrid);
    void increaseHValue();
    void decreaseHValue();
    void increasePencilSizeValue();
    void decreasePencilSizeValue();
    void setActiveClusterColor(const QColor* color);
    void setUndoRedo(bool undo, bool redo);

signals:
    void parametersChanged();
    void activeClusterChanged();
    void askForClusterCreation();
    void undo();
    void redo();
    void askForMerging(int pixelNumber);

private slots:

    void on_onf_pickup_clicked();
    void on_onf_addCluster_clicked();
    void on_onf_drawLimit_clicked();
    void on_onf_fill_clicked();
    void on_onf_freemove_clicked();
    void on_onf_centerOnCell_clicked();
    void on_rb_clusters_clicked();
    void on_rb_mns_clicked();
    void on_rb_density_clicked();
    void on_sb_activeCluster_valueChanged(int arg1);
    void on_dsb_height_valueChanged(double arg1);
    void on_onf_showClusters_clicked();
    void on_sb_pencilSize_valueChanged(int arg1);
    void on_onf_activeClusterColor_toggled(bool checked);

    void changeSingleStep(int button);
    void on_onf_merge_clicked();
};

#endif // ONF_ACTIONSEGMENTCROWNSOPTIONS_H
