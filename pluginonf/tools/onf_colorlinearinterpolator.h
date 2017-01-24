#ifndef ONF_COLORLINEARINTERPOLATOR_H
#define ONF_COLORLINEARINTERPOLATOR_H

#include <QColor>
#include <QMap>
#include <QGradient>

/**
 * @brief Use this class to get an intermediate color from a linear gradient
 */
class ONF_ColorLinearInterpolator
{
public:
    ONF_ColorLinearInterpolator();

    /**
     * @brief Construct the linear interpolator from a qt gradient
     */
    void constructFromQGradient(const QGradient &gradient);

    /**
     * @brief Add/Replace a color, key must be between 0 and 1.
     */
    void setKeyValueAt(double key, const QColor &value);

    /**
     * @brief Call it after you add all key and value.
     */
    void finalize();

    /**
     * @brief Return the intermediate color at key 'key', key must be between 0 and 1.
     */
    QColor intermediateColor(double key);

private:
    QMap<double, QColor>    m_gradients;
};

#endif // ONF_COLORLINEARINTERPOLATOR_H
