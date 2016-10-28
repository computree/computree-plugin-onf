#ifndef ONF_ACTIONAGGREGATEITEMS_H
#define ONF_ACTIONAGGREGATEITEMS_H


#include "views/actions/onf_actionaggregateitemsoptions.h"
#include "ct_actions/abstract/ct_abstractactionforgraphicsview.h"
#include "ct_itemdrawable/abstract/ct_abstractsingularitemdrawable.h"

#include <QRect>

class ONF_ActionAggregateItems : public CT_AbstractActionForGraphicsView
{
    Q_OBJECT
public:

    ONF_ActionAggregateItems(QStringList &modalities, QList<CT_AbstractSingularItemDrawable *> &items, QList<QString> &itemsModalities);

    ~ONF_ActionAggregateItems();

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

    void redraw();

public slots:
    void modalityChanged(QString modality);

private:
    QStringList                             *_modalities;
    QList<CT_AbstractSingularItemDrawable*> *_items;
    QList<QString>                          *_itemsModalities;

};


#endif // ONF_ACTIONAGGREGATEITEMS_H
