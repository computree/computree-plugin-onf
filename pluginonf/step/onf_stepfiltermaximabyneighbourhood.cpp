#include "onf_stepfiltermaximabyneighbourhood.h"

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"
#include "ct_itemdrawable/ct_image2d.h"
#include "ct_itemdrawable/ct_referencepoint.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"

// Alias for indexing models
#define DEFin_res "res"
#define DEFin_grp "grp"
#define DEFin_image "image"
#define DEFin_maxima "maxima"


// Constructor : initialization of parameters
ONF_StepFilterMaximaByNeighbourhood::ONF_StepFilterMaximaByNeighbourhood(CT_StepInitializeData &dataInit) : CT_AbstractStep(dataInit)
{
    _createMaximaPts = true;
}

// Step description (tooltip of contextual menu)
QString ONF_StepFilterMaximaByNeighbourhood::getStepDescription() const
{
    return tr("Filtrer les maxima par voisinage");
}

// Step detailled description
QString ONF_StepFilterMaximaByNeighbourhood::getStepDetailledDescription() const
{
    return tr("No detailled description for this step");
}

// Step URL
QString ONF_StepFilterMaximaByNeighbourhood::getStepURL() const
{
    //return tr("STEP URL HERE");
    return CT_AbstractStep::getStepURL(); //by default URL of the plugin
}

// Step copy method
CT_VirtualAbstractStep* ONF_StepFilterMaximaByNeighbourhood::createNewInstance(CT_StepInitializeData &dataInit)
{
    return new ONF_StepFilterMaximaByNeighbourhood(dataInit);
}

//////////////////// PROTECTED METHODS //////////////////

// Creation and affiliation of IN models
void ONF_StepFilterMaximaByNeighbourhood::createInResultModelListProtected()
{
    CT_InResultModelGroupToCopy *resIn_res = createNewInResultModelForCopy(DEFin_res, tr("Maxima"));
    resIn_res->setZeroOrMoreRootGroup();
    resIn_res->addGroupModel("", DEFin_grp, CT_AbstractItemGroup::staticGetType(), tr("Groupe"));
    resIn_res->addItemModel(DEFin_grp, DEFin_image, CT_Image2D<float>::staticGetType(), tr("Image (hauteurs)"));
    resIn_res->addItemModel(DEFin_grp, DEFin_maxima, CT_Image2D<qint32>::staticGetType(), tr("Maxima"));

}

// Creation and affiliation of OUT models
void ONF_StepFilterMaximaByNeighbourhood::createOutResultModelListProtected()
{
    CT_OutResultModelGroupToCopyPossibilities *resCpy_res = createNewOutResultModelToCopy(DEFin_res);
    if (resCpy_res != NULL)
    {
        resCpy_res->addItemModel(DEFin_grp, _filteredMaxima_ModelName, new CT_Image2D<qint32>(), tr("Maxima filtrés"));
        if (_createMaximaPts)
        {
            resCpy_res->addGroupModel(DEFin_grp, _filteredMaximaPtsGrp_ModelName, new CT_StandardItemGroup(), tr("Maxima filtrés (Pts)"));
            resCpy_res->addItemModel(_filteredMaximaPtsGrp_ModelName, _filteredMaximaPts_ModelName, new CT_ReferencePoint(), tr("Maximum"));
        }
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepFilterMaximaByNeighbourhood::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addFileChoice(tr("Fichier de paramètres"),CT_FileChoiceButton::OneExistingFile , "Fichier de paramètres (*.*)", _fileName);
    configDialog->addBool("", "", tr("Créer des points pour les maxima"), _createMaximaPts);
}

void ONF_StepFilterMaximaByNeighbourhood::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

    QMap<double, double> radii;

    if (_fileName.size() > 0)
    {
        QFile parameterFile(_fileName.first());
        if (parameterFile.exists() && parameterFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&parameterFile);

            while (!stream.atEnd())
            {
                QString line = stream.readLine();
                if (!line.isEmpty())
                {
                    QStringList values = line.split("\t");

                    if (values.size() > 1)
                    {
                        bool ok1, ok2;
                        double height = values.at(0).toDouble(&ok1);
                        double radius = values.at(1).toDouble(&ok2);

                        if (ok1 && ok2)
                        {
                            radii.insert(height, radius);
                        }
                    }
                }
            }
            parameterFile.close();
        }
    }

    if (!radii.contains(0)) {radii.insert(0, radii.first());}
    radii.insert(std::numeric_limits<double>::max(), radii.last());

    // COPIED results browsing
    CT_ResultGroupIterator itCpy_grp(res, this, DEFin_grp);
    while (itCpy_grp.hasNext() && !isStopped())
    {
        CT_StandardItemGroup* grp = (CT_StandardItemGroup*) itCpy_grp.next();
        CT_Image2D<qint32>* maximaIn = (CT_Image2D<qint32>*)grp->firstItemByINModelName(this, DEFin_maxima);
        CT_Image2D<float>* imageIn = (CT_Image2D<float>*)grp->firstItemByINModelName(this, DEFin_image);

        if (maximaIn != NULL)
        {
            Eigen::Vector2d min;
            maximaIn->getMinCoordinates(min);
            CT_Image2D<qint32>* filteredMaxima = new CT_Image2D<qint32>(_filteredMaxima_ModelName.completeName(), res, min(0), min(1), maximaIn->colDim(), maximaIn->linDim(), maximaIn->resolution(), maximaIn->level(), maximaIn->NA(), 0);
            grp->addItemDrawable(filteredMaxima);

            filteredMaxima->getMat() = maximaIn->getMat().clone();

            setProgress(20);

            // Get maxima coordinates list
            QMultiMap<qint32, Eigen::Vector3d*> maximaCoords;
            QMultiMap<double, qint32> maximaHeights;

            for (size_t xx = 0 ; xx < maximaIn->colDim() ; xx++)
            {
                for (size_t yy = 0 ; yy < maximaIn->linDim() ; yy++)
                {
                    qint32 maximaID = maximaIn->value(xx, yy);

                    if (maximaID > 0 && maximaID != maximaIn->NA())
                    {
                        Eigen::Vector3d* coords = new Eigen::Vector3d();
                        if (maximaIn->getCellCenterCoordinates(xx, yy, *coords))
                        {
                            (*coords)(2) = imageIn->value(xx, yy);
                            maximaCoords.insert(maximaID, coords);
                            maximaHeights.insert((*coords)(2), maximaID);
                        }
                    }
                }
            }

            setProgress(25);


            // Compute ordered vector of maxima ids
            QList<qint32> validMaxima;

            QMapIterator<double, qint32> itH(maximaHeights);
            itH.toBack();
            while (itH.hasPrevious())
            {
                itH.previous();
                qint32 cl = itH.value();
                if (!validMaxima.contains(cl)) {validMaxima.append(cl);}
            }

            QVector<qint32> orderedMaxima = validMaxima.toVector();
            int mxSize = orderedMaxima.size();
            validMaxima.clear();

            // Create maxima coords vector
            QVector<Eigen::Vector3d> coords(mxSize);
            for (int i = 0 ; i < mxSize ; i++)
            {
                qint32 id = orderedMaxima.at(i);

                QList<Eigen::Vector3d*> coordinates = maximaCoords.values(id);
                coords[i] = *(coordinates.at(0));

                // Compute position of the current maxima if more than one pixel
                int size = coordinates.size();
                if (size > 1)
                {
                    for (int j = 1 ; j < size ; j++)
                    {
                        Eigen::Vector3d* pos = coordinates.at(j);
                        coords[i](0) += (*pos)(0);
                        coords[i](1) += (*pos)(1);
                        if ((*pos)(2) > coords[i](2)) {coords[i](2) = (*pos)(2);}
                    }

                    coords[i](0) /= size;
                    coords[i](1) /= size;
                }
            }


            setProgress(30);


            // For each radius, test others
            for (int i = 0 ; i < mxSize ; i++)
            {
                qint32 id = orderedMaxima.at(i);

                if (id > 0)
                {
                    double x = coords[i](0);
                    double y = coords[i](1);
                    double z = coords[i](2);
                    double radius = getRadius(z, radii);

                    // detect the maximum to remove
                    for (int j = i + 1 ; j < mxSize ; j++)
                    {
                        qint32 idTested = orderedMaxima.at(j);

                        if (idTested > 0)
                        {
                            double dist = sqrt(pow(x - coords[j](0), 2) + pow(y - coords[j](1), 2));
                            if (dist < radius)
                            {
                                orderedMaxima[j] = 0;
                            }
                        }
                    }
                }

                setProgress(29.0*(float)i / (float)mxSize + 30.0);
            }

            setProgress(60);

            for (int i = 0 ; i < mxSize ; i++)
            {
                qint32 cl = orderedMaxima.at(i);
                if (cl > 0)
                {
                    validMaxima.append(cl);

                    double x = coords[i](0);
                    double y = coords[i](1);
                    double z = coords[i](2);

                    if (_createMaximaPts)
                    {
                        CT_ReferencePoint* refPoint = new CT_ReferencePoint(_filteredMaximaPts_ModelName.completeName(), res, x, y, z, 0);
                        CT_StandardItemGroup* grpPt = new CT_StandardItemGroup(_filteredMaximaPtsGrp_ModelName.completeName(), res);
                        grpPt->addItemDrawable(refPoint);
                        grp->addGroup(grpPt);
                    }
                }
            }

            setProgress(70);


            QMap<qint32, qint32> newIds;
            qint32 cpt = 1;
            // effectively delete toRemove maximum and numbers them in a continuous way
            for (size_t xx = 0 ; xx < filteredMaxima->colDim() ; xx++)
            {
                for (size_t yy = 0 ; yy < filteredMaxima->linDim() ; yy++)
                {
                    qint32 maximaID = filteredMaxima->value(xx, yy);

                    if (maximaID > 0)
                    {
                        if (validMaxima.contains(maximaID))
                        {
                            qint32 newId = newIds.value(maximaID, 0);
                            if (newId == 0)
                            {
                                newId = cpt++;
                                newIds.insert(maximaID, newId);
                            }
                            filteredMaxima->setValue(xx, yy, newId);
                        } else {
                            filteredMaxima->setValue(xx, yy, 0);
                        }
                    }
                }
            }
            newIds.clear();
            setProgress(90);

            filteredMaxima->computeMinMax();

            qDeleteAll(maximaCoords.values());
            setProgress(99);
        }
    }
    setProgress(100);
}

double ONF_StepFilterMaximaByNeighbourhood::getRadius(double height, const QMap<double, double> &radii)
{
    double radius = 0;
    bool stop = false;
    QMapIterator<double, double> it(radii);
    while (it.hasNext() && !stop)
    {
        it.next();
        double h = it.key();
        double r = it.value();

        if (height >= h)
        {
            radius = r;
        } else {
            stop = true;
        }
    }

    return radius;
}
