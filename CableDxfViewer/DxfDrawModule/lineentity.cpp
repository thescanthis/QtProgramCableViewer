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

void LineEntity::GroupPolyLineCheck(DxfModel &model)
{
    for (DxfPolyline& poly : model.polylines) {
        if (poly.points.size() < 3)
            continue;

        QPainterPath path;
        path.moveTo(QPointF(poly.points[0].x, poly.points[0].y));
        for (int i = 1; i < poly.points.size(); ++i)
            path.lineTo(QPointF(poly.points[i].x, poly.points[i].y));
        path.closeSubpath();

        for (const DxfText& text : model.texts) {
            QPointF pt(text.pos.x, text.pos.y);
            if (path.contains(pt)) {
                poly.isGroupRegion = true;
                break;
            }
        }
    }
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

    if (m_currentPolylineIndex >= 0) {
        auto& poly = m_model.polylines[m_currentPolylineIndex];
        poly.points.push_back({ data.x, data.y });

        // 👇 정점이 3개 이상일 때 암묵적 closed 판단
        if (!poly.closed && poly.points.size() >= 3) {
            const DxfPoint& first = poly.points.front();
            const DxfPoint& last  = poly.points.back();

            if (qFuzzyCompare(first.x, last.x) &&
                qFuzzyCompare(first.y, last.y)) {
                poly.closed = true;
            }
        }
    }
}
