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

#ifndef ONF_ACTIONPICKITEMSINLIST_H
#define ONF_ACTIONPICKITEMSINLIST_H


#include "views/actions/onf_actionpickitemsinlistoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"

#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"

#include <QRect>

class ONF_ActionPickItemsInList : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionPickItemsInList(const QList<CT_AbstractItemDrawable*> &itemList, QList<CT_AbstractItemDrawable*> *selectedItems, float maxDist = 1);

    ~ONF_ActionPickItemsInList();

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

    CT_AbstractItemDrawable* addItemToSelection(const QPoint &point);
    CT_AbstractItemDrawable* removeItemFromSelection(const QPoint &point);

public slots:
    void redraw();
    void selectAll();
    void selectNone();

private:
    float       _maxDist;
    QPoint      _oldPos;
    Qt::MouseButtons  _buttonsPressed;

    QList<CT_AbstractItemDrawable*> _itemList;
    QList<CT_AbstractItemDrawable*> *_selectedItems;
};


#endif // ONF_ACTIONPICKITEMSINLIST_H
