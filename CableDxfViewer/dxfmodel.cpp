#include "pch.h"
#include "dxfmodel.h"

void DxfModel::clear()
{
    lines.clear();
    circles.clear();
    arcs.clear();
    polylines.clear();
    texts.clear();
}

bool DxfModel::empty() const
{
    return lines.empty()
        && circles.empty()
        && arcs.empty()
        && polylines.empty()
        && texts.empty();
}

QRectF DxfModel::computeBoundingRect() const
{
    if (empty())
        return QRectF();

    double minX =  1e30;
    double minY =  1e30;
    double maxX = -1e30;
    double maxY = -1e30;

    auto expandPoint = [&](const DxfPoint& p) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    };

    for (const auto& L : lines) {
        expandPoint(L.p1);
        expandPoint(L.p2);
    }

    // 원 (중심 ± 반지름)
    for (const auto& C : circles) {
        DxfPoint p1{ C.center.x - C.radius, C.center.y - C.radius };
        DxfPoint p2{ C.center.x + C.radius, C.center.y + C.radius };
        expandPoint(p1);
        expandPoint(p2);
    }

    // 호 (일단 전체 원으로 근사, 필요하면 더 정확히 수정 가능)
    for (const auto& A : arcs) {
        DxfPoint p1{ A.center.x - A.radius, A.center.y - A.radius };
        DxfPoint p2{ A.center.x + A.radius, A.center.y + A.radius };
        expandPoint(p1);
        expandPoint(p2);
    }

    // 폴리라인
    for (const auto& P : polylines) {
        for (const auto& pt : P.points) {
            expandPoint(pt);
        }
    }

    // 텍스트 위치도 박스에 포함시키고 싶으면 여기에 추가
    for (const auto& T : texts) {
        expandPoint(T.pos);
    }

    // DXF 좌표계 그대로 반환 (y뒤집기는 뷰에서 처리)
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

DxfModel DxfModel::filteredByLayer(const QStringList &layerWhitelist) const
{
    // 화이트리스트가 비어 있으면 전체 복사
    if (layerWhitelist.isEmpty())
        return *this;

    DxfModel out;

    auto keep = [&](const QString& layer) {
        return layerWhitelist.contains(layer);
    };

    for (const auto& L : lines)
        if (keep(L.layer)) out.lines.push_back(L);

    for (const auto& C : circles)
        if (keep(C.layer)) out.circles.push_back(C);

    for (const auto& A : arcs)
        if (keep(A.layer)) out.arcs.push_back(A);

    for (const auto& P : polylines)
        if (keep(P.layer)) out.polylines.push_back(P);

    for (const auto& T : texts)
        if (keep(T.layer)) out.texts.push_back(T);

    return out;
}
