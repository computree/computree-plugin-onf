#ifndef ONF_STEPFILTERMAXIMABYNEIGHBOURHOOD_H
#define ONF_STEPFILTERMAXIMABYNEIGHBOURHOOD_H

#ifdef USE_OPENCV

#include "ct_step/abstract/ct_abstractstep.h"

// Inclusion of auto-indexation system
#include "ct_tools/model/ct_autorenamemodels.h"
#include "ct_math/delaunay2d/ct_delaunaytriangulation.h"
#include "ct_itemdrawable/ct_image2d.h"


class ONF_StepFilterMaximaByNeighbourhood: public CT_AbstractStep
{
    Q_OBJECT

public:

    /*! \brief Step constructor
     * 
     * Create a new instance of the step
     * 
     * \param dataInit Step parameters object
     */
    ONF_StepFilterMaximaByNeighbourhood(CT_StepInitializeData &dataInit);

    /*! \brief Step description
     * 
     * Return a description of the step function
     */
    QString getStepDescription() const;

    /*! \brief Step detailled description
     * 
     * Return a detailled description of the step function
     */
    QString getStepDetailledDescription() const;

    /*! \brief Step URL
     * 
     * Return a URL of a wiki for this step
     */
    QString getStepURL() const;

    /*! \brief Step copy
     * 
     * Step copy, used when a step is added by step contextual menu
     */
    CT_VirtualAbstractStep* createNewInstance(CT_StepInitializeData &dataInit);

protected:

    /*! \brief Input results specification
     * 
     * Specification of input results models needed by the step (IN)
     */
    void createInResultModelListProtected();

    /*! \brief Parameters DialogBox
     * 
     * DialogBox asking for step parameters
     */
    void createPostConfigurationDialog();

    /*! \brief Output results specification
     * 
     * Specification of output results models created by the step (OUT)
     */
    void createOutResultModelListProtected();

    /*! \brief Algorithm of the step
     * 
     * Step computation, using input results, and creating output results
     */
    void compute();

private:

    QStringList _fileName;
    bool        _createMaximaPts;
    double      _scoreThreshold;

    // Declaration of autoRenames Variables (groups or items added to In models copies)
    CT_AutoRenameModels    _filteredMaxima_ModelName;
    CT_AutoRenameModels    _filteredMaximaPtsGrp_ModelName;
    CT_AutoRenameModels    _filteredMaximaPts_ModelName;

    double computeScore(CT_Image2D<float> *heightImage, CT_DelaunayVertex *baseVertex, CT_DelaunayVertex *neighbourVertex);
    double getRadius(double height, const QMap<double, double> &radii);
};

#endif

#endif // ONF_STEPFILTERMAXIMABYNEIGHBOURHOOD_H
