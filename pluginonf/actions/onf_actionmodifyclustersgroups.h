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

#ifndef ONF_ACTIONMODIFYCLUSTERSGROUPS_H
#define ONF_ACTIONMODIFYCLUSTERSGROUPS_H

#include <QRect>

#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"
#include "ct_itemdrawable/ct_point2d.h"
#include "ct_itemdrawable/ct_pointcluster.h"

/**
 * @brief Action to select items, points, faces, edges, etc...
 *
 * You can use this action in your action if you want. Just call methods :
 *
 * init()
 * mousePressEvent()
 * etc.....
 *
 * in methods of your action.
 *
 * Dont forget to set the selection mode => setSelectionMode method
 */
class ONF_ActionModifyClustersGroups : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionModifyClustersGroups(QMap<const CT_Point2D*, QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > > *map);

    QString uniqueName() const;
    QString title() const;
    QString description() const;
    QIcon icon() const;
    QString type() const;

    void init();

    void updateAllClustersColors();
    void updateColorForOneCluster(const CT_Point2D *position);

    void updateVisiblePositions();

    bool mousePressEvent(QMouseEvent *e);
    bool mouseMoveEvent(QMouseEvent *e);
    bool mouseReleaseEvent(QMouseEvent *e);

    bool keyPressEvent(QKeyEvent *e);
    bool keyReleaseEvent(QKeyEvent *e);

    void drawOverlay(GraphicsViewInterface &view, QPainter &painter);

    CT_AbstractAction* copy() const;

    /**
     * @brief change the selection mode for this action. The mode will be set to view
     *        when the mousePressEvent() method is called
     * @return false if we are between mousePressEvent() and mouseReleaseEvent()
     */
    bool setSelectionMode(GraphicsViewInterface::SelectionMode mode);

    GraphicsViewInterface::SelectionMode selectionMode() const;

public slots:
    void setColorA(QColor color);
    void setColorB(QColor color);
    void selectPositionA();
    void selectPositionB();
    void swapAandB();
    void visibilityChanged();
    void affectClusterToA();
    void affectClusterToB();
    void affectClusterToTMP();
    void affectClusterToTrash();
    void updateLimitMode();
    void distanceChanged(int val);

private:
    QRect                                   m_selectionRectangle;
    int                                     m_status;
    GraphicsViewInterface::SelectionMode    m_selectionMode;
    bool                                    m_mousePressed;

    QMap<const CT_Point2D*, QPair<CT_PointCloudIndexVector*, QList<const CT_PointCluster*>* > > *_positionToCluster;
    QMap<const CT_PointCluster*, const CT_Point2D*> _clusterToPosition;

    QList<QColor>                   _automaticColorList;
    QMap<const CT_Point2D*, QColor> _positionsBaseColors;
    QColor                          _colorA;
    QColor                          _colorB;
    QColor                          _colorTmp;
    QColor                          _colorTrash;

    CT_Point2D*                     _positionA;
    CT_Point2D*                     _positionB;
    QList<CT_PointCluster*>         _temporaryClusterList;
    QList<CT_PointCluster*>         _trashClusterList;
    bool                            _ABColors;
    bool                            _positionsChanged;
    QList<CT_PointCluster*>         _clustersOrdered;
    int                             _currentLastA;



    GraphicsViewInterface::SelectionMode selectionModeToBasic(GraphicsViewInterface::SelectionMode mode) const;

    void addToA(CT_PointCluster* cluster);
    void addToB(CT_PointCluster* cluster);

};

#endif // ONF_ACTIONMODIFYCLUSTERSGROUPS_H
