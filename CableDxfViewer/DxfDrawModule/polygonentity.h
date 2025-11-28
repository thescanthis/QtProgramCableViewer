#ifndef POLYGONENTITY_H
#define POLYGONENTITY_H

#include "dxfmodel.h"

class DL_CircleData;
class DL_Attributes;
class DxfBlockResolver;

class PolygonEntity
{
public:
    PolygonEntity(DxfModel& model, DxfBlockResolver& resolver);

    void addCircle(const DL_CircleData& data, const DL_Attributes& attr);

    // TODO: 나중에 Rect 관련 함수 추가 가능
    // void addRectFromPolyline(const DxfPolyline& poly, ...);

private:
    DxfModel&         m_model;
    DxfBlockResolver& m_blockResolver;
};

#endif // POLYGONENTITY_H
