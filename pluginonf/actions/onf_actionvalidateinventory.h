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

#ifndef ONF_ACTIONVALIDATEINVENTORY_H
#define ONF_ACTIONVALIDATEINVENTORY_H


#include "views/actions/onf_actionvalidateinventoryoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"

#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"
#include "ct_itemdrawable/ct_standarditemgroup.h"

#include <QRect>

class ONF_ActionValidateInventory : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionValidateInventory(QMap<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*> *selectedItem,
                             QMultiMap<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*> *availableItem,
                             QMap<const CT_StandardItemGroup*, QString> *species,
                             QMap<const CT_StandardItemGroup*, QString> *ids,
                             const QStringList &speciesList);

    ~ONF_ActionValidateInventory();

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

    const CT_AbstractSingularItemDrawable *chooseForItem(const QPoint &point);
    const CT_AbstractSingularItemDrawable *chooseForAttributes(const QPoint &point);
    void setAttributes(const CT_StandardItemGroup *scene);

public slots:
    void redraw();
    void updateSpeciesList(const QString &species);
    void resetCurrentItem();

private:
    QPoint      _oldPos;
    Qt::MouseButtons  _buttonsPressed;
    const CT_AbstractSingularItemDrawable* _currentItem;


    QMap<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*>           *_selectedItem;
    QMap<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*>           _initiallySelectedItem;
    QMultiMap<const CT_StandardItemGroup*, const CT_AbstractSingularItemDrawable*>      *_availableItem;
    QMap<const CT_StandardItemGroup*, QString>                                          *_species;
    QMap<const CT_StandardItemGroup*, QString>                                          *_ids;
    QStringList                                                                         _speciesList;
};


#endif // ONF_ACTIONVALIDATEINVENTORY_H
