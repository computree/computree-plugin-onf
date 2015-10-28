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

#ifndef ONF_ACTIONSELECTCELLSINGRID3DCOLONIZEDIALOG_H
#define ONF_ACTIONSELECTCELLSINGRID3DCOLONIZEDIALOG_H

#include <QDialog>

namespace Ui {
class ONF_ActionSelectCellsInGrid3DColonizeDialog;
}

class ONF_ActionSelectCellsInGrid3DColonizeDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ONF_ActionSelectCellsInGrid3DColonizeDialog(QWidget *parent = 0);
    ~ONF_ActionSelectCellsInGrid3DColonizeDialog();

    void setXp(bool val);
    void setXm(bool val);
    void setYp(bool val);
    void setYm(bool val);
    void setZp(bool val);
    void setZm(bool val);

    bool getXp();
    bool getXm();
    bool getYp();
    bool getYm();
    bool getZp();
    bool getZm();

private:
    Ui::ONF_ActionSelectCellsInGrid3DColonizeDialog *ui;

};

#endif // ONF_ACTIONSELECTCELLSINGRID3DCOLONIZEDIALOG_H
