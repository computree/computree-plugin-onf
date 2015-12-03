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

#ifndef ONF_ACTIONMODIFYPOSITIONS2D_H
#define ONF_ACTIONMODIFYPOSITIONS2D_H


#include "views/actions/onf_actionmodifypositions2doptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"

#include "ct_itemdrawable/ct_point2d.h"
#include "ct_result/ct_resultgroup.h"

#include <QRect>
#include <QTime>

class ONF_ActionModifyPositions2D : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionModifyPositions2D(QList<CT_Point2D*> &positions, CT_OutAbstractSingularItemModel *model, CT_ResultGroup *outRes);

    ~ONF_ActionModifyPositions2D();

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

    void updatePlane();
public slots:
    void zValChanged();
private:
    Qt::MouseButtons                    _buttonsPressed;
    QList<CT_Point2D*>*                 _positions;
    CT_Point2D*                         _selectedPoint;
    CT_OutAbstractSingularItemModel*    _model;
    CT_ResultGroup*                     _outRes;
    bool                                _leftButton;

    QColor                              _selectedColor;
    QColor                              _normalColor;
    Eigen::Vector2d                     _min;
    Eigen::Vector2d                     _max;    
    double                              _zmin;
    double                              _zmax;


    bool getCoordsForMousePosition(const QMouseEvent *e, double &x, double &y);
    CT_Point2D *getNearestPosition(double x, double y);
};


#endif // ONF_ACTIONMODIFYPOSITIONS2D_H
