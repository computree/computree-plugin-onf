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

#include "onf_filterbyreturntype.h"
#include "ct_pointcloudindex/ct_pointcloudindexvector.h"
#include "ct_iterator/ct_pointiterator.h"

ONF_FilterByReturnType::ONF_FilterByReturnType() : CT_AbstractFilter_LAS()
{
    _type = ONF_FilterByReturnType::All;
    _typeAsString = "All";

    _filterByClassif = false;
    _keepVegetation = true;
    _keepGround = false;
    _keepNotClassified = false;
    _keepBuldings = false;
    _keepWater = false;
    _keepValues = "";
}

ONF_FilterByReturnType::ONF_FilterByReturnType(const ONF_FilterByReturnType *other) : CT_AbstractFilter_LAS(other)
{
    _type = other->_type;
    _typeAsString = getStringForType(_type);
}

QString ONF_FilterByReturnType::getName()
{
    return QString("PBF_ReturnType");
}

QString ONF_FilterByReturnType::getCompleteName()
{
    QString result = getStringForType(_type);
    if (_filterByClassif)
    {
        if (_keepVegetation) {result.append("Veg");}
        if (_keepGround) {result.append("Gnd");}
        if (_keepNotClassified) {result.append("Ncl");}
        if (_keepBuldings) {result.append("Bul");}
        if (_keepWater) {result.append("Wat");}
        if (!_keepValues.isEmpty())
        {
            result.append("Clas");
            result.append(_keepValues.replace(";", "."));
        }
    }

    return result;
}

void ONF_FilterByReturnType::createConfigurationDialog()
{
    CT_StepConfigurableDialog* configDialog = addConfigurationDialog();

    QStringList typesList;
    for (int type = ONF_FilterByReturnType::ReturnType_begin ; type <= ONF_FilterByReturnType::ReturnType_end ; type++)
    {
        typesList.append(getStringForType((ONF_FilterByReturnType::ReturnType) type));
    }

    _typeAsString = getStringForType(_type);

    configDialog->addStringChoice(tr("Type de retours à conserver"), "", typesList, _typeAsString);
    configDialog->addEmpty();
    configDialog->addBool(tr("Filter selon la classification"), "", tr("Conserver les classifications suivantes :"), _filterByClassif);
    configDialog->addBool("", "", tr("Végétation (3,4,5)"), _keepVegetation);
    configDialog->addBool("", "", tr("Sol (2)"), _keepGround);
    configDialog->addBool("", "", tr("Non classés (0,1)"), _keepNotClassified);
    configDialog->addBool("", "", tr("Constructions (6)"), _keepBuldings);
    configDialog->addBool("", "", tr("Eau (9)"), _keepWater);
    configDialog->addString(tr("Autres valeurs à conserver (séparées par des ;)"), "", _keepValues);
}

void ONF_FilterByReturnType::updateParamtersAfterConfiguration()
{
    _type = getTypeForString(_typeAsString);

    _classifToKeep.clear();
    if (_filterByClassif)
    {
        if (_keepNotClassified) {_classifToKeep.insert(0);_classifToKeep.insert(1);}
        if (_keepGround) {_classifToKeep.insert(2);}
        if (_keepVegetation) {_classifToKeep.insert(3); _classifToKeep.insert(4); _classifToKeep.insert(5);}
        if (_keepBuldings) {_classifToKeep.insert(6);}
        if (_keepWater) {_classifToKeep.insert(9);}

        QStringList vals = _keepValues.split(";", QString::SkipEmptyParts);
        for (int i =  0 ; i < vals.size() ; i++)
        {
            bool ok = false;
            int val = vals.at(i).toInt(&ok);

            if (ok && val >=0 && val <= 255)
            {
                _classifToKeep.insert(val);
            }
        }
    }
}

QString ONF_FilterByReturnType::getParametersAsString() const
{
    QString result = "";
    result.append(getStringForType(_type));

    if (_filterByClassif)
    {
        result.append(";");
        QSetIterator<quint8> it(_classifToKeep);
        while (it.hasNext())
        {
            result.append(QString("%1;").arg(it.next()));
        }
    }

    return result;
}

bool ONF_FilterByReturnType::setParametersFromString(QString parameters)
{
    _classifToKeep.clear();

    QStringList params = parameters.split(";", QString::SkipEmptyParts);
    if (params.size() > 0)
    {
        _type = getTypeForString(params.at(0));
    }

    if (params.size() > 1)
    {
        _filterByClassif = true;
        for (int i = 1 ; i < params.size() ; i++)
        {
            bool ok = false;
            int val = params.at(i).toInt(&ok);

            if (ok && val >=0 && val <= 255)
            {
                _classifToKeep.insert(val);
            }
        }
    }


    _keepNotClassified = _classifToKeep.contains(0) || _classifToKeep.contains(1);
    _keepGround = _classifToKeep.contains(2);
    _keepVegetation = _classifToKeep.contains(3) || _classifToKeep.contains(4) || _classifToKeep.contains(5);
    _keepBuldings = _classifToKeep.contains(6);
    _keepWater = _classifToKeep.contains(9);

    QSetIterator<quint8> it(_classifToKeep);
    while (it.hasNext())
    {
        quint8 val = it.next();
        if (val > 6 && val != 9)
        {

        }
        _keepValues.append(QString("%1;").arg(val));
    }

    return true;
}

QString ONF_FilterByReturnType::getShortDescription() const
{
    return tr("Filter by return type and Classification");
}

QString ONF_FilterByReturnType::getDetailledDescription() const
{
    return tr("Filter by return type. You can specify following types:\n"
              "- First: first returns\n"
              "- Last: last returns\n"
              "- LastAndOnly: last and only returns\n"
              "- Intermediare: Returns which are not first neither last\n"
              "- Only: first returns if no other return for the ray\n"
              "- All : don't filter on return type\n"
              "\n"
              "You can also choose which classifications to keep.");
}

CT_AbstractConfigurableElement *ONF_FilterByReturnType::copy() const
{
    ONF_FilterByReturnType* filter = new ONF_FilterByReturnType(this);
    filter->_type = _type;
    filter->_typeAsString = _typeAsString;

    filter->_filterByClassif = _filterByClassif;

    if (_filterByClassif)
    {
        QSetIterator<quint8> it(_classifToKeep);
        while (it.hasNext())
        {
            filter->_classifToKeep.insert(it.next());
        }
    }

    return filter;
}

void ONF_FilterByReturnType::validatePoint(CT_PointIterator &pointIt, CT_LASData &lasData) const
{   
    // Test for the type of return
    if (_type == ONF_FilterByReturnType::First && lasData._Return_Number != 1) {return;}
    if (_type == ONF_FilterByReturnType::Last && (lasData._Return_Number != lasData._Number_of_Returns || lasData._Return_Number == 1)) {return;}
    if (_type == ONF_FilterByReturnType::LastAndOnly && lasData._Return_Number != lasData._Number_of_Returns) {return;}
    if (_type == ONF_FilterByReturnType::Intermediate && (lasData._Return_Number == 1 || lasData._Return_Number == lasData._Number_of_Returns)) {return;}
    if (_type == ONF_FilterByReturnType::Only && (lasData._Return_Number != 1 || lasData._Number_of_Returns != 1)) {return;}

    // test for classification
    if (_filterByClassif && !_classifToKeep.contains(lasData._Classification)) {return;}

    _outCloud->addIndex(pointIt.currentGlobalIndex());
}

QString ONF_FilterByReturnType::getStringForType(ONF_FilterByReturnType::ReturnType returnType) const
{
    switch (returnType)
    {
        case ONF_FilterByReturnType::First : return "first";
        case ONF_FilterByReturnType::Last : return "last";
        case ONF_FilterByReturnType::LastAndOnly : return "lastAndOnly";
        case ONF_FilterByReturnType::Intermediate : return "int";
        case ONF_FilterByReturnType::Only : return "only";
    }

    return "all";
}

ONF_FilterByReturnType::ReturnType ONF_FilterByReturnType::getTypeForString(QString returnTypeAsString) const
{
    if (returnTypeAsString == "first") {return ONF_FilterByReturnType::First;}
    if (returnTypeAsString == "last") {return ONF_FilterByReturnType::Last;}
    if (returnTypeAsString == "lastAndOnly") {return ONF_FilterByReturnType::LastAndOnly;}
    if (returnTypeAsString == "int") {return ONF_FilterByReturnType::Intermediate;}
    if (returnTypeAsString == "only") {return ONF_FilterByReturnType::Only;}

    return ONF_FilterByReturnType::All;
}
