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

#ifndef ONF_ACTIONMODIFYAFFILIATIONS_H
#define ONF_ACTIONMODIFYAFFILIATIONS_H

#include "views/actions/onf_actionmodifyaffiliationsoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"

#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"

#include <QRect>

class ONF_ActionModifyAffiliations : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:
    ONF_ActionModifyAffiliations(const QList<CT_AbstractSingularItemDrawable*> *sourceList,
                                const QList<CT_AbstractSingularItemDrawable*> *targetList,
                                QMap<CT_AbstractSingularItemDrawable*, CT_AbstractSingularItemDrawable*> *correspondances);

    QString uniqueName() const;
    QString title() const;
    QString description() const;
    QIcon icon() const;
    QString type() const;

    void init();

    bool mouseDoubleClickEvent(QMouseEvent *e);
    bool mousePressEvent(QMouseEvent *e);
    bool mouseMoveEvent(QMouseEvent *e);
    bool mouseReleaseEvent(QMouseEvent *e);
    bool wheelEvent(QWheelEvent *e);

    bool keyPressEvent(QKeyEvent *e);
    bool keyReleaseEvent(QKeyEvent *e);

    void draw(GraphicsViewInterface &view, PainterInterface &painter);
    void drawOverlay(GraphicsViewInterface &view, QPainter &painter);

    CT_AbstractAction* copy() const;

    CT_AbstractSingularItemDrawable * closetItemFromClickDirection(QMouseEvent *e,
                                                                   const QList<CT_AbstractSingularItemDrawable *> *itemList);
public slots:
    void updateGraphics();
    void affiliate();
    void breakAffiliation();

private:
    int         m_status;

    float       _symbolSize;

    const QList<CT_AbstractSingularItemDrawable*>     *_sourceList;
    const QList<CT_AbstractSingularItemDrawable*>     *_targetList;
    QMap<CT_AbstractSingularItemDrawable*, CT_AbstractSingularItemDrawable*>    *_correspondances; // (key=target, value=source)
    QMap<CT_AbstractSingularItemDrawable*, Eigen::Vector3d>   _itemCenters;
    CT_AbstractSingularItemDrawable*                    _activeSource;
    CT_AbstractSingularItemDrawable*                    _activeTarget;


    void drawSourceItem(PainterInterface &painter, const Eigen::Vector3d &position);
    void drawTargetItem(PainterInterface &painter, const Eigen::Vector3d &position);
    void drawAffiliation(PainterInterface &painter, const Eigen::Vector3d &source, const Eigen::Vector3d &target);

};


#endif // ONF_ACTIONMODIFYAFFILIATIONS_H
