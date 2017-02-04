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
#include "ct_itemdrawable/ct_standarditemgroup.h"
#include "ct_itemdrawable/abstract/ct_abstractitemdrawablewithpointcloud.h"
#include "ctliblas/itemdrawable/las/ct_stdlaspointsattributescontainer.h"
#include "ct_result/abstract/ct_abstractresult.h"

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
        _trueheight = 0;
        _idPlot = "";
        _idTree = "";
        _cyl = NULL;
        _grp = NULL;
    }

    ~ONF_ActionAdjustPlotPosition_treePosition()
    {
        _cyl = NULL;
        _grp = NULL;
    }

    double                  _x;
    double                  _y;
    float                   _dbh;
    float                   _height;
    float                   _trueheight;
    QString                 _idPlot;
    QString                 _idTree;
    CT_Cylinder*            _cyl;
    CT_StandardItemGroup*   _grp;
};

class ONF_ActionAdjustPlotPosition_dataContainer
{
public:
    ONF_ActionAdjustPlotPosition_dataContainer();
    ~ONF_ActionAdjustPlotPosition_dataContainer()
    {
        _positions.clear();
        _scenes.clear();
        _LASattributes.clear();
    }

    QList<ONF_ActionAdjustPlotPosition_treePosition*>   _positions;
    QList<CT_AbstractItemDrawable*>                     _scenes;
    QList<CT_StdLASPointsAttributesContainer*>          _LASattributes;
    double                                              _transX;
    double                                              _transY;
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
    void update(double x, double y, bool circles, bool fixedH, double h);
    void updateColorization(bool colorizeByIntensity, int min, int max);
    void applyTranslation(bool reset);
    void setGradient(bool intensity, QString name, int min, int max);
    void changeHighlightedNumber(int n);

private:

    ONF_ActionAdjustPlotPosition_dataContainer* _dataContainer;
    QList<CT_AbstractItemDrawable*>             _cylinders;

    ONF_AdjustPlotPositionCylinderDrawManager*  _drawManager;

    ONF_ColorLinearInterpolator                _gradientGrey;
    ONF_ColorLinearInterpolator                _gradientHot;
    ONF_ColorLinearInterpolator                _gradientRainbow;
    ONF_ColorLinearInterpolator                _gradientHSV;

    ONF_ColorLinearInterpolator                _currentZGradient;
    ONF_ColorLinearInterpolator                _currentIGradient;

    QColor                                     _cylinderColor;
    QColor                                     _selectedCylinderColor;
    QColor                                     _highlightedCylindersColor;

    double _minZ;
    double _maxZ;
    double _rangeZ;
    double _minI;
    double _maxI;
    double _rangeI;
    bool   _colorizeByIntensity;

    Qt::MouseButtons                                _buttonsPressed;
    QPoint                                          _lastPos;
    ONF_ActionAdjustPlotPosition_treePosition*      _selectedPos;
    Eigen::Vector3d                                 _currentPoint;
    bool                                            _movePlot;


    void colorizePoints(ONF_ColorLinearInterpolator &gradient, int min, int max);    
    ONF_ActionAdjustPlotPosition_treePosition* getPositionFromPoint(Eigen::Vector3d &point);    
    ONF_ActionAdjustPlotPosition_treePosition *getPositionFromDirection(Eigen::Vector3d &origin, Eigen::Vector3d &direction);
    bool getCoordsForMousePosition(QPoint p, double &x, double &y);


    static bool lessThan(CT_AbstractItemDrawable* i1, CT_AbstractItemDrawable* i2)
    {
        CT_Cylinder* cyl1 = dynamic_cast<CT_Cylinder*>(i1);
        CT_Cylinder* cyl2 = dynamic_cast<CT_Cylinder*>(i2);

        if (cyl1 == NULL || cyl2 == NULL ) {return true;}

        return cyl1->getRadius() > cyl2->getRadius();
    }

private slots:
    void redrawOverlay();
    void redrawOverlayAnd3D();


};


#endif // ONF_ACTIONADJUSTPLOTPOSITION_H
