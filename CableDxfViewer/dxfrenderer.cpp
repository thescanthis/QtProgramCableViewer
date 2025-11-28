#include "pch.h"
#include "dxfrenderer.h"

#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QPen>
#include <QFont>


void DxfRenderer::render(QGraphicsScene *scene, const DxfModel &model)
{
    if (!scene)
        return;

    scene->clear();

    QPen pen(Qt::black);
    pen.setWidthF(0);    // 뷰 스케일에 따라 항상 가는 선

    // 1) 선 / 폴리라인 / 원
    renderLines(scene, model, pen);
    renderPolylines(scene, model, pen);
    renderCircles(scene, model, pen);

    // 2) 텍스트(TEXT / MTEXT)
    renderTexts(scene, model);
}

// -------------------------
// LINE (단일 선분 목록)
// -------------------------
void DxfRenderer::renderLines(QGraphicsScene *scene, const DxfModel &model, const QPen &pen)
{
    for (const auto& L : model.lines)
    {
        scene->addLine(L.p1.x, -L.p1.y,
                       L.p2.x, -L.p2.y,
                       pen);
    }
}

// -------------------------
// POLYLINE
// -------------------------
void DxfRenderer::renderPolylines(QGraphicsScene *scene, const DxfModel &model, const QPen &pen)
{
    for (const auto& P : model.polylines)
    {
        if (P.points.size() < 2)
            continue;

        // 연속 선분
        for (size_t i = 0; i + 1 < P.points.size(); ++i)
        {
            const auto& a = P.points[i];
            const auto& b = P.points[i + 1];
            scene->addLine(a.x, -a.y,
                           b.x, -b.y,
                           pen);
        }

        // 닫힌 도형이면 마지막-첫번째도 이어주기
        if (P.closed && P.points.size() >= 3)
        {
            const auto& a = P.points.back();
            const auto& b = P.points.front();
            scene->addLine(a.x, -a.y,
                           b.x, -b.y,
                           pen);
        }
    }
}

// -------------------------
// CIRCLE
// -------------------------
void DxfRenderer::renderCircles(QGraphicsScene *scene, const DxfModel &model, const QPen &pen)
{
    for (const auto& C : model.circles)
    {
        double r    = C.radius;
        double left = C.center.x - r;
        double top  = -(C.center.y + r);
        double size = r * 2.0;

        scene->addEllipse(left, top, size, size, pen);
    }
}

// -------------------------
// TEXT + MTEXT
// -------------------------
void DxfRenderer::renderTexts(QGraphicsScene *scene, const DxfModel &model)
{
    for (const auto& T : model.texts)
    {
        renderSingleText(scene, T);
    }
}

// TEXT 하나에 대한 실제 렌더링
void DxfRenderer::renderSingleText(QGraphicsScene *scene, const DxfText &t)
{
    if (t.text.isEmpty())
        return;

    auto* item = scene->addText(t.text);

    // 폰트 설정: DXF height를 적당히 pointSize로 사용
    QFont f = item->font();
    double h = t.height;
    if (h <= 0.0) h = 12.0;
    if (h > 100.0) h = 100.0;
    f.setPointSizeF(h);
    item->setFont(f);

    // 중앙 정렬: boundingRect 기준으로 좌표 보정
    QRectF br = item->boundingRect();
    double x = t.pos.x - br.width()  / 2.0;
    double y = -t.pos.y - br.height() / 2.0;

    item->setPos(x, y);
}
