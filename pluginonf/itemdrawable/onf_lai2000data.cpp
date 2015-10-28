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


#include "onf_lai2000data.h"

#include "math.h"

ONF_Lai2000Data::ONF_Lai2000Data() : CT_AbstractItemDrawableWithoutPointCloud()
{
    _t1 = 0;
    _t2 = 0;
    _t3 = 0;
    _t4 = 0;
    _t5 = 0;
    _LAI3A = 0;
}

ONF_Lai2000Data::ONF_Lai2000Data(const CT_OutAbstractSingularItemModel *model,
                               const CT_AbstractResult *result,
                               double t1,
                               double t2,
                               double t3,
                               double t4,
                               double t5) : CT_AbstractItemDrawableWithoutPointCloud(model, result)
{
    _t1 = t1;
    _t2 = t2;
    _t3 = t3;
    _t4 = t4;
    _t5 = t5;

    _LAI3A = -2 * (log(_t1)*cos(ANGLE1m)*W1 + log(_t2)*cos(ANGLE2m)*W2 + log(_t3)*cos(ANGLE3m)*(W3 + W4 + W5));
}

ONF_Lai2000Data::ONF_Lai2000Data(const QString &modelName,
                               const CT_AbstractResult *result,
                               double t1,
                               double t2,
                               double t3,
                               double t4,
                               double t5) : CT_AbstractItemDrawableWithoutPointCloud(modelName, result)
{
    _t1 = t1;
    _t2 = t2;
    _t3 = t3;
    _t4 = t4;
    _t5 = t5;

    _LAI3A = -2 * (log(_t1)*cos(ANGLE1m)*W1 + log(_t2)*cos(ANGLE2m)*W2 + log(_t3)*cos(ANGLE3m)*(W3 + W4 + W5));
}

QString ONF_Lai2000Data::getType() const
{
    return staticGetType();
}

QString ONF_Lai2000Data::staticGetType()
{
    return CT_AbstractItemDrawableWithoutPointCloud::staticGetType() + "/ONF_Lai2000Data";
}

int ONF_Lai2000Data::nDataAvalaible() const
{
    return 6;
}

QVariant ONF_Lai2000Data::data(int column) const
{
    if(column == 0)
        return QString("T1=%1").arg(_t1);
    if(column == 1)
        return QString("T2=%1").arg(_t2);
    if(column == 2)
        return QString("T3=%1").arg(_t3);
    if(column == 3)
        return QString("T4=%1").arg(_t4);
    if(column == 4)
        return QString("T5=%1").arg(_t5);
    if(column == 5)
        return QString("LAI3A=%1").arg(_LAI3A);
    return 0;
}

CT_AbstractItemDrawable* ONF_Lai2000Data::copy(const CT_OutAbstractItemModel *model, const CT_AbstractResult *result, CT_ResultCopyModeList copyModeList)
{
    ONF_Lai2000Data *cpy = new ONF_Lai2000Data((const CT_OutAbstractSingularItemModel *)model, result, _t1, _t2, _t3, _t4, _t5);
    cpy->setId(id());
    return cpy;
}
