#include "pch.h"
#include "lineentity.h"
#include "dxfblockresolver.h"
#include "dl_entities.h"
#include "dl_attributes.h"

LineEntity::LineEntity(DxfModel &model, DxfBlockResolver &resolver)
    : m_model(model)
    , m_blockResolver(resolver)
{
}

// -------------------------
// LINE 처리
// -------------------------
void LineEntity::addLine(const DL_LineData &data,
                         const DL_Attributes &attr)
{
    // BLOCK 안에 있으면 BlockResolver에 맡김
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addLine(data, attr);
        return;
    }

    // ---- 전역 LINE ----
    DxfLine L;
    L.p1   = { data.x1, data.y1 };
    L.p2   = { data.x2, data.y2 };
    L.layer = QString::fromStdString(attr.getLayer());

    m_model.lines.push_back(L);
}

// -------------------------
// POLYLINE 시작
// -------------------------
void LineEntity::addPolyline(const DL_PolylineData &data,
                             const DL_Attributes &attr)
{
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addPolyline(data, attr);
        return;
    }

    // ---- 전역 POLYLINE ----
    DxfPolyline P;
    P.closed = (data.flags & 1);
    P.layer  = QString::fromStdString(attr.getLayer());

    m_model.polylines.push_back(std::move(P));
    m_currentPolylineIndex =
        static_cast<int>(m_model.polylines.size()) - 1;
}

// -------------------------
// POLYLINE의 각 Vertex 추가
// -------------------------
void LineEntity::addVertex(const DL_VertexData &data)
{
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addVertex(data);
        return;
    }

    if (m_currentPolylineIndex < 0 ||
        m_currentPolylineIndex >= static_cast<int>(m_model.polylines.size()))
        return;

    DxfPoint pt{ data.x, data.y };
    m_model.polylines[m_currentPolylineIndex].points.push_back(pt);
}
