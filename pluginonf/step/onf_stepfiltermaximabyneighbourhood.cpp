#include "onf_stepfiltermaximabyneighbourhood.h"

#ifdef USE_OPENCV

#include "ct_itemdrawable/tools/iterator/ct_groupiterator.h"
#include "ct_result/ct_resultgroup.h"
#include "ct_result/model/inModel/ct_inresultmodelgrouptocopy.h"
#include "ct_result/model/outModel/tools/ct_outresultmodelgrouptocopypossibilities.h"
#include "ct_view/ct_stepconfigurabledialog.h"
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
    _scoreThreshold = 0.5;
    _minRadius = 1.5;
    _maxRadius = 10.0;

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
    }
}

// Semi-automatic creation of step parameters DialogBox
void ONF_StepFilterMaximaByNeighbourhood::createPostConfigurationDialog()
{
    CT_StepConfigurableDialog *configDialog = newStandardPostConfigurationDialog();
    configDialog->addDouble(tr("DeltaH maximum dans un houppier"), "m", 0, 10, 2, _scoreThreshold);
    configDialog->addDouble(tr("Rayon de houppier minimal"), "m", 0, 1000, 2, _minRadius);
    configDialog->addDouble(tr("Rayon de houppier maximal"), "m", 0, 1000, 2, _maxRadius);
}

void ONF_StepFilterMaximaByNeighbourhood::compute()
{
    QList<CT_ResultGroup*> outResultList = getOutResultList();
    CT_ResultGroup* res = outResultList.at(0);

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

            bool filtered = true;
            while (filtered)
            {
                // Compute triangulation
                CT_DelaunayTriangulation *delaunay = new CT_DelaunayTriangulation();
                delaunay->init(maximaIn->minX(), maximaIn->minY(), maximaIn->maxX(), maximaIn->maxY());

                QVector<CT_DelaunayVertex*> delaunayVertices(mxSize);

                for (int i = 0 ; i < coords.size() ; i++)
                {
                    if (orderedMaxima.at(i) > 0)
                    {
                        delaunayVertices[i] = delaunay->addVertex(&coords[i], false);
                    }
                }
                delaunay->doInsertion();
                delaunay->computeVerticesNeighbors();

                setProgress(50);

                filtered = false;
                for (int i = 0 ; i < delaunayVertices.size() && !filtered; i++)
                {
                    if (orderedMaxima.at(i) > 0)
                    {
                        CT_DelaunayVertex* baseVertex = delaunayVertices.at(i);
                        QList<CT_DelaunayVertex*> &neighbours = baseVertex->getNeighbors();

                        for (int j = 0 ; j < neighbours.size() ; j++)
                        {
                            CT_DelaunayVertex* neighbour = neighbours.at(j);

                            double score = computeScore(imageIn, baseVertex, neighbour);

                            if (score < _scoreThreshold)
                            {
                                orderedMaxima[delaunayVertices.indexOf(neighbour)] = 0;
                                filtered = true;
                            }
                        }
                    }
                }
                delete delaunay;
            }



            setProgress(60);

            for (int i = 0 ; i < mxSize ; i++)
            {
                qint32 cl = orderedMaxima.at(i);

                if (cl > 0) {validMaxima.append(cl);}
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

double ONF_StepFilterMaximaByNeighbourhood::computeScore(CT_Image2D<float>* heightImage, CT_DelaunayVertex* baseVertex, CT_DelaunayVertex* neighbourVertex)
{
    double dist = sqrt(pow(baseVertex->x() - neighbourVertex->x(), 2) + pow(baseVertex->y() - neighbourVertex->y(), 2));

    if (dist <= _minRadius) {return 0;}
    if (dist >= _maxRadius) {return std::numeric_limits<double>::max();}

    Eigen::Vector3d dir = *(neighbourVertex->getData()) - *(baseVertex->getData());

    double offset =  1 / (dist / (heightImage->resolution() / 10.0));
    double deltaHMax = 0;

    for (double l = 0 ; l < 1 ; l += offset)
    {
        Eigen::Vector3d pos = *(baseVertex->getData()) + dir*l;
        double h = heightImage->valueAtCoords(pos(0), pos(1));
        double deltaH = pos(2) - h;

        if (deltaH > deltaHMax) {deltaHMax = deltaH;}
    }

    return deltaHMax;
}



#endif
