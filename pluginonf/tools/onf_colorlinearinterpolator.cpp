#include "onf_colorlinearinterpolator.h"

ONF_ColorLinearInterpolator::ONF_ColorLinearInterpolator()
{
}

void ONF_ColorLinearInterpolator::constructFromQGradient(const QGradient &gradient)
{
    m_gradients.clear();

    QGradientStops stops = gradient.stops();

    if(!stops.isEmpty())
    {
        QGradientStop lastStop;
        lastStop.first = -1;

        QGradientStop firstStop;
        firstStop.first = 2;

        QVectorIterator< QGradientStop > it(stops);

        while(it.hasNext())
        {
            const QGradientStop &stop = it.next();

            setKeyValueAt(stop.first, stop.second);

            if(stop.first > lastStop.first)
                lastStop = stop;

            if(stop.first < firstStop.first)
                firstStop = stop;
        }
    }

    finalize();
}

void ONF_ColorLinearInterpolator::setKeyValueAt(double key, const QColor &value)
{
    m_gradients.insert(key, value);
}

void ONF_ColorLinearInterpolator::finalize()
{
    if(m_gradients.isEmpty())
    {
        m_gradients.insert(0, Qt::white);
        m_gradients.insert(1, Qt::white);
        return;
    }

    if(!m_gradients.contains(0))
    {
        QMap<double, QColor>::const_iterator it = m_gradients.begin();
        m_gradients.insert(0, it.value());
    }

    if(!m_gradients.contains(1))
    {
        QMap<double, QColor>::const_iterator it = m_gradients.end();
        m_gradients.insert(1, it.value());
    }
}

QColor ONF_ColorLinearInterpolator::intermediateColor(double key)
{
    QMap<double, QColor>::const_iterator it = m_gradients.lowerBound(key);

    QMap<double, QColor>::const_iterator itB = it;
    QMap<double, QColor>::const_iterator itN = it;

    if(it == m_gradients.begin())
        return it.value();
    else if(it.key() > key)
        itB = it-1;
    else if(it == m_gradients.end())
        return it.value();
    else if(it.key() == key)
        itN = it+1;

    double max = itN.key()-itB.key();
    double fraction = ((double)key-itB.key())/max;

    float RED1 = itB.value().redF();
    float GREEN1 = itB.value().greenF();
    float BLUE1 = itB.value().blueF();
    float ALPHA1 = itB.value().alphaF();

    float DELTA_RED = itN.value().redF() - RED1;
    float DELTA_GREEN = itN.value().greenF() - GREEN1;
    float DELTA_BLUE = itN.value().blueF() - BLUE1;
    float DELTA_ALPHA = itN.value().alphaF() - ALPHA1;

    float red = RED1 + (DELTA_RED * fraction);
    float green = GREEN1 + (DELTA_GREEN * fraction);
    float blue = BLUE1 + (DELTA_BLUE * fraction);
    float alpha = ALPHA1 + (DELTA_ALPHA * fraction);

    QColor color;
    color.setRgbF(red, green, blue, alpha);

    return color;
}
