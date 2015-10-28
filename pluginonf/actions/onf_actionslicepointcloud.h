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

#ifndef ONF_ACTIONSLICEPOINTCLOUD_H
#define ONF_ACTIONSLICEPOINTCLOUD_H


#include "views/actions/onf_actionslicepointcloudoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"
#include "ct_itemdrawable/ct_scene.h"

#include <QRect>

class ONF_ActionSlicePointCloud_dataContainer
{
public:
    ONF_ActionSlicePointCloud_dataContainer();
    double            _thickness;
    double            _spacing;
    double            _zBase;
};

class ONF_ActionSlicePointCloud : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionSlicePointCloud(QList<CT_Scene*>* sceneList, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax, ONF_ActionSlicePointCloud_dataContainer* dataContainer);

    ~ONF_ActionSlicePointCloud();

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
    void update();

private:
    QList<CT_Scene*>* _sceneList;

    float             _xmin;
    float             _ymin;
    float             _zmin;
    float             _xmax;
    float             _ymax;
    float             _zmax;

    float             _xwidth;
    float             _ywidth;

    ONF_ActionSlicePointCloud_dataContainer* _dataContainer;

private slots:
    void redrawOverlay();
    void redrawOverlayAnd3D();

};


#endif // ONF_ACTIONSLICEPOINTCLOUD_H
