#include "pch.h"
#include "polygonentity.h"

#include "dxfblockresolver.h"
#include "dl_entities.h"
#include "dl_attributes.h"

PolygonEntity::PolygonEntity(DxfModel &model, DxfBlockResolver &resolver)
    : m_model(model)
    , m_blockResolver(resolver)
{
}

void PolygonEntity::addCircle(const DL_CircleData &data,
                              const DL_Attributes &attr)
{
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addCircle(data, attr);
        return;
    }

    DxfCircle C;
    C.center = { data.cx, data.cy };
    C.radius = data.radius;
    C.layer  = QString::fromStdString(attr.getLayer());

    m_model.circles.push_back(C);
}
