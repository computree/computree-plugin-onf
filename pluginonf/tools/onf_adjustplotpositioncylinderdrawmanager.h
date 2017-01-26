#ifndef ONF_ADJUSTPLOTPOSITIONCYLINDERDRAWMANAGER_H
#define ONF_ADJUSTPLOTPOSITIONCYLINDERDRAWMANAGER_H

#include "ct_itemdrawable/tools/drawmanager/ct_standardabstractshapedrawmanager.h"
#include "ct_itemdrawable/ct_cylinder.h"

class ONF_AdjustPlotPositionCylinderDrawManager : public CT_StandardAbstractShapeDrawManager
{
public:
    ONF_AdjustPlotPositionCylinderDrawManager(QString drawConfigurationName = "");
    virtual ~ONF_AdjustPlotPositionCylinderDrawManager();

    virtual void draw(GraphicsViewInterface &view, PainterInterface &painter, const CT_AbstractItemDrawable &itemDrawable) const;

    void setTranslation(double x, double y);
    void setParameters(bool circles, bool fixedH, double h);
    void setColor(QColor color);
    void setSelectionColor(QColor color);
    void setselectedCylinder(CT_Cylinder* selectedCylinder);

private:
    double  _transX;
    double  _transY;
    QColor  _color;
    QColor  _selectionColor;
    bool _circles;
    bool _fixedH;
    double _h;

    CT_Cylinder*    _selectedCylinder;

protected:

    virtual CT_ItemDrawableConfiguration createDrawConfiguration(QString drawConfigurationName) const;
};

#endif // ONF_ADJUSTPLOTPOSITIONCYLINDERDRAWMANAGER_H
