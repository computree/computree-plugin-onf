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

#ifndef ONF_ACTIONADJUSTPLOTPOSITION_H
#define ONF_ACTIONADJUSTPLOTPOSITION_H


#include "views/actions/onf_actionadjustplotpositionoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"
#include "ct_itemdrawable/ct_cylinder.h"
#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"

#include "tools/onf_adjustplotpositioncylinderdrawmanager.h"
#include "tools/onf_colorlinearinterpolator.h"


#include <QRect>

class ONF_ActionAdjustPlotPosition_treePosition
{
public:
    ONF_ActionAdjustPlotPosition_treePosition()
    {
        _x = 0;
        _y = 0;
        _dbh = 0;
        _height = 0;
        _idPlot = "";
        _idTree = "";
    }

    double           _x;
    double           _y;
    float            _dbh;
    float            _height;
    QString          _idPlot;
    QString          _idTree;
};

class ONF_ActionAdjustPlotPosition_dataContainer
{
public:
    ONF_ActionAdjustPlotPosition_dataContainer();

    QList<ONF_ActionAdjustPlotPosition_treePosition*> _positions;
    QList<CT_AbstractItemDrawableWithPointCloud*>     _scenes;
    double              _transX;
    double              _transY;
};

class ONF_ActionAdjustPlotPosition : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionAdjustPlotPosition(ONF_ActionAdjustPlotPosition_dataContainer* dataContainer);

    ~ONF_ActionAdjustPlotPosition();

    QString uniqueName() const;
    QString title() const;
    QString description() const;
    QIcon icon() const;
    QString type() const;

    void init();

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
    void update(double x = 0, double y = 0);

private:

    ONF_ActionAdjustPlotPosition_dataContainer* _dataContainer;
    QList<CT_AbstractItemDrawable*>             _cylinders;

    ONF_AdjustPlotPositionCylinderDrawManager*  _drawManager;

    ONF_ColorLinearInterpolator                _gradientGrey;
    ONF_ColorLinearInterpolator                _gradientRainbow;
    ONF_ColorLinearInterpolator                _gradientHot;

    double _minz;
    double _maxz;
    double _range;

    void colorizePoints(ONF_ColorLinearInterpolator &gradient);

private slots:
    void redrawOverlay();
    void redrawOverlayAnd3D();

};


#endif // ONF_ACTIONADJUSTPLOTPOSITION_H
