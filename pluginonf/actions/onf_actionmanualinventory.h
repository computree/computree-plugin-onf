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

#ifndef ONF_ACTIONMANUALINVENTORY_H
#define ONF_ACTIONMANUALINVENTORY_H


#include "views/actions/onf_actionmanualinventoryoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"

#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"
#include "ct_itemdrawable/ct_scene.h"
#include "ct_itemdrawable/ct_circle.h"

#include <QRect>

class ONF_ActionManualInventory : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionManualInventory(QMap<const CT_Scene*, const CT_Circle*> *selectedDbh,
                             QMap<const CT_Scene*, QMultiMap<double, const CT_Circle*> > *availableDbh,
                             QList<const CT_Circle*> *preferredDbh,
                             QList<const CT_Scene *> *trashedScenes,
                             QMap<const CT_Scene*, double> *sceneDTMValues,
                             QMap<QString, QStringList> *paramData,
                             QMap<const CT_Scene*, QMap<QString, QString> >  *suppAttributes);

    ~ONF_ActionManualInventory();

    QString uniqueName() const;
    QString title() const;
    QString description() const;
    QIcon icon() const;
    QString type() const;

    void init();

    bool mousePressEvent(QMouseEvent *e);
    bool mouseDoubleClickEvent(QMouseEvent *e);
    bool mouseMoveEvent(QMouseEvent *e);
    bool mouseReleaseEvent(QMouseEvent *e);
    bool wheelEvent(QWheelEvent *e);

    bool keyPressEvent(QKeyEvent *e);
    bool keyReleaseEvent(QKeyEvent *e);

    void draw(GraphicsViewInterface &view, PainterInterface &painter);
    void drawOverlay(GraphicsViewInterface &view, QPainter &painter);

    CT_AbstractAction* copy() const;

    void chooseForDbh(const QPoint &point);
    void chooseForAttributes(const QPoint &point);
    void selectActiveScene(const QPoint &point);
    void setAttributes(const CT_Scene *scene);

    void updateVisibility(CT_Scene *scene, CT_Circle *circle);
public slots:
    void visibilityChanged();
    void selectUpperCircle();
    void selectLowerCircle();
    void sendToTrash();
    void retreiveFromTrash();
    void sendToValidated();

private:
    CT_Circle*          _currentCircle;
    CT_Scene*           _currentScene;


    QMap<const CT_Scene*, const CT_Circle*>                         *_selectedDbh;
    QMap<const CT_Scene*, QMultiMap<double, const CT_Circle*> >     *_availableDbh;
    QList<const CT_Circle*>                                         *_preferredDbh;
    QList<const CT_Scene *>                                         *_trashedScenes;
    QMap<const CT_Scene *, double>                                  *_sceneDTMValues;

    QMap<QString, QStringList>                                      *_paramData;
    QMap<const CT_Scene*, QMap<QString, QString> >                  *_suppAttributes;

    QList<const CT_Scene *>                                         _validatedScenes;



    QColor              _othersScenesCirclesColor;
    QColor              _activeSceneCirclesColor;
    QColor              _activeSceneCirclesLightColor;
    QColor              _othersCircleColor;
    QColor              _othersScenesCirclesLightColor;
    QColor              _currentCircleColor;
    QColor              _trashActiveCircleColor;
    QColor              _trashOtherCircleColor;
    QColor              _validatedActiveCircleColor;
    QColor              _validatedOtherCircleColor;

};


#endif // ONF_ACTIONMANUALINVENTORY_H
