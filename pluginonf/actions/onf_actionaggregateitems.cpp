#include "actions/onf_actionaggregateitems.h"
#include "ct_global/ct_context.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QIcon>
#include <QPainter>

ONF_ActionAggregateItems::ONF_ActionAggregateItems(const QStringList &modalities, const QList<CT_AbstractSingularItemDrawable*> &items) : CT_AbstractActionForGraphicsView()
{
    _modalities.append(modalities);
    _items.append(items);

    for (int i = 0 ; i < _items.size() ; i++)
    {
        _itemsModalities.append("");
    }
}

ONF_ActionAggregateItems::~ONF_ActionAggregateItems()
{
    _modalities.clear();
    _items.clear();
    _itemsModalities.clear();
}

QString ONF_ActionAggregateItems::uniqueName() const
{
    return "ONF_ActionAggregateItems";
}

QString ONF_ActionAggregateItems::title() const
{
    return tr("Regroupement d'items");
}

QString ONF_ActionAggregateItems::description() const
{
    return tr("Regroupement d'items");
}

QIcon ONF_ActionAggregateItems::icon() const
{
    return QIcon(":/icons/select_rectangular.png");
}

QString ONF_ActionAggregateItems::type() const
{
    return CT_AbstractAction::TYPE_SELECTION;
}

void ONF_ActionAggregateItems::init()
{
    CT_AbstractActionForGraphicsView::init();

    if(nOptions() == 0)
    {
        // create the option widget if it was not already created
        ONF_ActionAggregateItemsOptions *option = new ONF_ActionAggregateItemsOptions(this);

        // add the options to the graphics view
        graphicsView()->addActionOptions(option);

        connect(option, SIGNAL(currentModalityChanged(QString)), this, SLOT(modalityChanged(QString)));

        for (int i = 0 ; i < _modalities.size() ; i++)
        {
            option->addModality(_modalities.at(i));
        }

        // register the option to the superclass, so the hideOptions and showOptions
        // is managed automatically
        registerOption(option);

        document()->redrawGraphics();
    }
}

void ONF_ActionAggregateItems::redraw()
{
    document()->redrawGraphics();
}

bool ONF_ActionAggregateItems::mousePressEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) && (e->modifiers()  & Qt::ControlModifier))
    {
        redraw();
        return true;
    }
    return false;
}

bool ONF_ActionAggregateItems::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionAggregateItems::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    return false;
}

bool ONF_ActionAggregateItems::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers()  & Qt::ControlModifier)
    {
        if (e->delta()>0)
        {
        }
        redraw();
        return true;
    }

    return false;
}

bool ONF_ActionAggregateItems::keyPressEvent(QKeyEvent *e)
{
    if((e->key() == Qt::Key_S) && !e->isAutoRepeat())
    {
        redraw();
        return true;
    }
    return false;
}

bool ONF_ActionAggregateItems::keyReleaseEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    return false;
}

void ONF_ActionAggregateItems::draw(GraphicsViewInterface &view, PainterInterface &painter)
{
    Q_UNUSED(view);
    Q_UNUSED(painter);
}

void ONF_ActionAggregateItems::drawOverlay(GraphicsViewInterface &view, QPainter &painter)
{
    Q_UNUSED(view);
    Q_UNUSED(painter);
}

CT_AbstractAction* ONF_ActionAggregateItems::copy() const
{
    return new ONF_ActionAggregateItems(_modalities, _items);
}

void ONF_ActionAggregateItems::modalityChanged(QString modality)
{
    const QList<CT_AbstractItemDrawable*>& itemList = graphicsView()->document()->getItemDrawable();

    for (int i = 0 ; i < itemList.size() ; i++)
    {
        CT_AbstractSingularItemDrawable* item = (CT_AbstractSingularItemDrawable*) itemList.at(i);
        int index = _items.indexOf(item);
        if (index > 0)
        {
            _itemsModalities[index] = modality;
        }
    }
}
