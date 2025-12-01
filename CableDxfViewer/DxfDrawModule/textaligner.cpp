#include "pch.h"
#include "textaligner.h"

TextAligner::TextAligner(DxfModel& model)
    : m_model(model)
{

}

void TextAligner::alignAll()
{
    alignInBoxes();
    alignInPolygons();
    alignInCircles();
}

void TextAligner::alignInBoxes()
{
    for (const DxfPolyline& poly : m_model.polylines) {
        if (!poly.closed || poly.points.size() != 9)
            continue; // 사각형이 아닐 경우 skip

        QRectF rect = getBoundingBox(poly.points);
        QPointF center = rect.center();

        for (DxfText& text : m_model.texts) {
            if (rect.contains(QPointF(text.pos.x, text.pos.y))) {
                text.pos = DxfPoint{center.x(),center.y()};
                text.align = TextAlign::Center;
            }
        }
    }
}

void TextAligner::alignInPolygons()
{
    for (const DxfPolyline& poly : m_model.polylines) {
        if (!poly.closed || poly.points.size() < 5)
            continue; // 사각형 아닌 다각형

        QRectF rect = getBoundingBox(poly.points);
        QPointF center = rect.center();

        for (DxfText& text : m_model.texts) {
            if (rect.contains(QPointF(text.pos.x, text.pos.y))) {
                text.pos = DxfPoint{center.x(),center.y()};
                text.align = TextAlign::Center;
            }
        }
    }
}

void TextAligner::alignInCircles()
{
    for (const DxfCircle& circle : m_model.circles) {
        QPointF center(circle.center.x, circle.center.y);

        for (DxfText& text : m_model.texts) {
            QPointF diff = QPointF{text.pos.x,text.pos.y} - center;
            if (std::hypot(diff.x(), diff.y()) <= circle.radius) {
                text.pos = DxfPoint{center.x(),center.y()};
                text.align = TextAlign::Center;
            }
        }
    }
}

QRectF TextAligner::getBoundingBox(const std::vector<DxfPoint> &points)
{
    qreal minX = points[0].x, maxX = points[0].x;
    qreal minY = points[0].y, maxY = points[0].y;

    for (const DxfPoint& p : points) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

std::vector<QPointF> TextAligner::toQPoints(const std::vector<DxfPoint> &dxfPoints)
{
    std::vector<QPointF> qPoints;
    qPoints.reserve(dxfPoints.size());

    for (const auto& p : dxfPoints)
        qPoints.emplace_back(p.x, p.y);

    return qPoints;
}
