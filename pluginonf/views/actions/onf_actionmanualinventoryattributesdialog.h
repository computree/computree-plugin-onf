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

#ifndef ONF_ACTIONMANUALINVENTORYATTRIBUTESDIALOG_H
#define ONF_ACTIONMANUALINVENTORYATTRIBUTESDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QMap>

namespace Ui {
class ONF_ActionManualInventoryAttributesDialog;
}

class ONF_ActionManualInventoryAttributesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ONF_ActionManualInventoryAttributesDialog(QMap<QString, QStringList> *paramData,
                                                      QMap<QString, QString> &attrValues,
                                                      QWidget *parent = 0);
    ~ONF_ActionManualInventoryAttributesDialog();

    QString getValueForAttr(QString name) const;

private:
    Ui::ONF_ActionManualInventoryAttributesDialog *ui;

    QMap<QString, QStringList>                      *_paramData;
    QMap<QString, QString>                          *_attrValues;
    QMap<QString, QWidget*>                         _widgets;

};

#endif // ONF_ACTIONMANUALINVENTORYATTRIBUTESDIALOG_H
