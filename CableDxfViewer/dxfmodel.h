#ifndef DXFMODEL_H
#define DXFMODEL_H
#pragma once

struct DxfPoint
{
    double x = 0.0;
    double y = 0.0;
};

struct DxfLine
{
    DxfPoint p1;
    DxfPoint p2;
    QString  layer;
};

struct DxfCircle
{
    DxfPoint center;
    double   radius = 0.0;
    QString  layer;
};

struct DxfArc
{
    DxfPoint center;
    double   radius      = 0.0;
    double   startAngle  = 0.0;  // degree 또는 rad, 통일하기
    double   endAngle    = 0.0;
    QString  layer;
};

struct DxfPolyline
{
    std::vector<DxfPoint> points;
    bool     closed = false;
    QString  layer;
};

enum class DxfEntityType
{
    Line,
    Circle,
    Arc,
    Polyline,
    Text
};

struct DxfText
{
    DxfPoint pos;
    QString  text;
    double   height = 0.0;
    QString  layer;
};

class DxfModel
{
public:
    DxfModel() = default;
    DxfModel(std::vector<DxfLine> lines, std::vector<DxfCircle> circles,
             std::vector<DxfArc> arcs, std::vector<DxfPolyline> polylines,
             std::vector<DxfText> texts)
        : lines(std::move(lines)), circles(std::move(circles)),
        arcs(std::move(arcs)), polylines(std::move(polylines)),
        texts(std::move(texts)) {}
    ~DxfModel() = default;

    std::vector<DxfLine>     lines;
    std::vector<DxfCircle>   circles;
    std::vector<DxfArc>      arcs;
    std::vector<DxfPolyline> polylines;
    std::vector<DxfText>     texts;

    // 전체 비우기
    void clear();

    // 아무 엔티티도 없는지
    bool empty() const;

    // 전체 도면의 바운딩 박스 (DXF 좌표계 기준)
    QRectF computeBoundingRect() const;

    // 레이어 화이트리스트 기준 필터링 (DIM/TEXT 제거 같은 용도)
    DxfModel filteredByLayer(const QStringList& layerWhitelist) const;
};

#endif // DXFMODEL_H
