#ifndef DXFRENDERER_H
#define DXFRENDERER_H

#include "dxfmodel.h"

class QGraphicsScene;
class QPen;
class QFont;

/// DxfModel을 QGraphicsScene에 그려주는 전담 클래스
/// - 내부에 Line / Polyline / Circle / Text / MText용 함수 분리
class DxfRenderer
{
public:
    DxfRenderer() = default;

    // 전체 렌더링 진입점
    void render(QGraphicsScene* scene, const DxfModel& model);

private:
    // 개별 엔티티 렌더링 함수들
    void renderLines(QGraphicsScene* scene, const DxfModel& model, const QPen& pen);
    void renderPolylines(QGraphicsScene* scene, const DxfModel& model, const QPen& pen);
    void renderCircles(QGraphicsScene* scene, const DxfModel& model, const QPen& pen);
    void renderTexts(QGraphicsScene* scene, const DxfModel& model);

    // 텍스트 하나 그리기 (TEXT / MTEXT 공용)
    void renderSingleText(QGraphicsScene* scene, const DxfText& t);
};

#endif // DXFRENDERER_H
