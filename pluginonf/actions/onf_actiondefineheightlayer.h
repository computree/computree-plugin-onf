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

#ifndef ONF_ACTIONDEFINEHEIGHTLAYER_H
#define ONF_ACTIONDEFINEHEIGHTLAYER_H


#include "views/actions/onf_actiondefineheightlayeroptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"

#include "ct_itemdrawable/ct_grid2dxy.h"
#include "ct_itemdrawable/ct_scene.h"


#include <QRect>

class ONF_ActionDefineHeightLayer_gridContainer
{
public:
    ONF_ActionDefineHeightLayer_gridContainer();

    CT_Grid2DXY<int>    *_densityGrid;
    CT_Grid2DXY<double>  *_mnsGrid;
    double              _zmin;
    double              _zmax;
};

class ONF_ActionDefineHeightLayer : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionDefineHeightLayer();
    ONF_ActionDefineHeightLayer(const CT_AbstractResult *result, const QString &densityGridModel, const QString &mnsGridModel, ONF_ActionDefineHeightLayer_gridContainer *gridContainer, const QList<CT_Scene*> &list, double xmin, double ymin, double zmin, double xmax, double ymax, double zmax);

    void createGrids(double res);

    QString uniqueName() const;
    QString title() const;
    QString description() const;
    QIcon icon() const;
    QString type() const;

    void init();

    double getZmin() const;
    double getZmax() const;

    bool mousePressEvent(QMouseEvent *e);
    bool mouseMoveEvent(QMouseEvent *e);
    bool mouseReleaseEvent(QMouseEvent *e);
    bool wheelEvent(QWheelEvent *e);

    bool keyPressEvent(QKeyEvent *e);
    bool keyReleaseEvent(QKeyEvent *e);

    void draw(GraphicsViewInterface &view, PainterInterface &painter);
    void drawOverlay(GraphicsViewInterface &view, QPainter &painter);

    CT_AbstractAction* copy() const;

public slots:
    void updateGraphics();
    void setSideView();
    void computeCrownProjection();
    void updateZValues(double zmin, double zmax);
    void updateResolution(double resolution);
    void updateThreshold(int threshold);

private:
    int         m_status;

    CT_AbstractResult *_result;
    QString _densityGridModel;
    QString _mnsGridModel;

    ONF_ActionDefineHeightLayer_gridContainer *_gridContainer;

    QList<CT_Scene*>     _sceneList;
    double               _previousResolution;
    double               _resolution;
    int                 _densityThreshold;
    double               _xmin;
    double               _ymin;
    double               _xmax;
    double               _ymax;
    double               _xrange;
    double               _yrange;

};


#endif // ONF_ACTIONDEFINEHEIGHTLAYER_H
