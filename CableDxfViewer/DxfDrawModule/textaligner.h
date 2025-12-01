#ifndef TEXTALIGNER_H
#define TEXTALIGNER_H

#include "../dxfmodel.h"

class TextAligner
{
public:
    explicit TextAligner(DxfModel& model);

    void alignAll();

private:
    void alignInBoxes();
    void alignInPolygons();
    void alignInCircles();

    QRectF getBoundingBox(const std::vector<DxfPoint>& points);
    std::vector<QPointF> toQPoints(const std::vector<DxfPoint>& dxfPoints);

private:
    DxfModel& m_model;

};

#endif // TEXTALIGNER_H
