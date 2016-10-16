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


#include "onf_lvoxsphere.h"
#include "qdebug.h"

#include "math.h"

CT_DEFAULT_IA_INIT(ONF_LvoxSphere)

ONF_LvoxSphere::ONF_LvoxSphere() : CT_AbstractItemDrawableWithoutPointCloud()
{
}

ONF_LvoxSphere::ONF_LvoxSphere(const CT_OutAbstractSingularItemModel *model,
                             const CT_AbstractResult *result,
                             QString name,
                             float xs,
                             float ys,
                             float zs,
                             float rs,
                             float re) : CT_AbstractItemDrawableWithoutPointCloud(model, result)
{
    _name = name;
    _xs = xs;
    _ys = ys;
    _zs = zs;
    _rs = rs;
    _re = re;
    _ni = 0;
    _nb = 0;
    _nt = 0;
    _ne = 0;

    setCenterX(_xs);
    setCenterY(_ys);
    setCenterZ(_zs);
}

ONF_LvoxSphere::ONF_LvoxSphere(const QString &modelName,
                             const CT_AbstractResult *result,
                             QString name,
                             float xs,
                             float ys,
                             float zs,
                             float rs,
                             float re) : CT_AbstractItemDrawableWithoutPointCloud(modelName, result)
{
    _name = name;
    _xs = xs;
    _ys = ys;
    _zs = zs;
    _rs = rs;
    _re = re;
    _ni = 0;
    _nb = 0;
    _nt = 0;
    _ne = 0;

    setCenterX(_xs);
    setCenterY(_ys);
    setCenterZ(_zs);
}

CT_AbstractItemDrawable* ONF_LvoxSphere::copy(const CT_OutAbstractItemModel *model, const CT_AbstractResult *result, CT_ResultCopyModeList copyModeList)
{
    ONF_LvoxSphere *sphere = new ONF_LvoxSphere((const CT_OutAbstractSingularItemModel *)model, result, _name, _xs, _ys, _zs, _rs, _re);
    sphere->setId(id());
    sphere->setNi(_ni);
    sphere->setNb(_nb);
    sphere->setNt(_nt);
    sphere->setNe(_ne);
    return sphere;
}

void ONF_LvoxSphere::setNi(int ni)
{
    _ni = ni;
}

void ONF_LvoxSphere::setNb(int nb)
{
    _nb = nb;
}

void ONF_LvoxSphere::setNt(float nt)
{
    _nt = nt;
}

void ONF_LvoxSphere::setNe(int ne)
{
    _ne = ne;
}
QString ONF_LvoxSphere::getName() const
{
    return _name;
}

float ONF_LvoxSphere::getXs() const
{
    return _xs;
}

float ONF_LvoxSphere::getYs() const
{
    return _ys;
}

float ONF_LvoxSphere::getZs() const
{
    return _zs;
}

float ONF_LvoxSphere::getRs() const
{
    return _rs;
}

float ONF_LvoxSphere::getRe() const
{
    return _re;
}

int ONF_LvoxSphere::getNi() const
{
    return _ni;
}

int ONF_LvoxSphere::getNb() const
{
    return _nb;
}

float ONF_LvoxSphere::getNt() const
{
    return _nt;
}

int ONF_LvoxSphere::getNe() const
{
    return _ne;
}


