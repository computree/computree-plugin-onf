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

#ifndef ONF_FILTERBYRETURNTYPE_H
#define ONF_FILTERBYRETURNTYPE_H

#include "ctlibio/filters/las/abstract/ct_abstractfilter_las.h"

class ONF_FilterByReturnType : public CT_AbstractFilter_LAS
{
    Q_OBJECT
public:

    enum ReturnType {
        First,
        Intermediate,
        Last,
        LastAndOnly,
        Only,
        All,
        ReturnType_begin = First,
        ReturnType_end = All
    };

    ONF_FilterByReturnType();
    ONF_FilterByReturnType(const ONF_FilterByReturnType* other);

    QString getName();
    QString getCompleteName();

    void createConfigurationDialog();
    void updateParamtersAfterConfiguration();

    QString getShortDescription() const;
    QString getDetailledDescription() const;

    QString getParametersAsString() const;
    virtual bool setParametersFromString(QString parameters);

    CT_AbstractConfigurableElement* copy() const;

    void validatePoint(CT_PointIterator& pointIt, CT_LASData &LADData) const;


private:

    ONF_FilterByReturnType::ReturnType   _type;
    QString                             _typeAsString;

    QSet<quint8>   _classifToKeep;
    bool        _filterByClassif;
    bool        _keepVegetation;
    bool        _keepGround;
    bool        _keepNotClassified;
    bool        _keepBuldings;
    bool        _keepWater;
    QString     _keepValues;


    QString getStringForType(ONF_FilterByReturnType::ReturnType returnType) const;
    ONF_FilterByReturnType::ReturnType getTypeForString(QString returnTypeAsString) const;
};

#endif // ONF_FILTERBYRETURNTYPE_H
