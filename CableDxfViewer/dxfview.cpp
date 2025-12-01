#include "pch.h"
#include "dxfview.h"
#include "dxfloader.h"
#include "textaligner.h"
#include "lineentity.h"

#include <QGraphicsScene>
#include <QWheelEvent>
#include <QPen>
#include <QDebug>

DxfView::DxfView(QWidget* parent)
    :QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);

    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
}

bool DxfView::loadDxfFile(const QString &filePath)
{
    QString err;
    if (!dxfloader::LoadFile(filePath, m_model, err)) {
        qWarning() << "DXF load failed:" << err;
        m_scene->clear();
        return false;
    }

    LineEntity::GroupPolyLineCheck(m_model);
    TextAligner aligner(m_model);
    aligner.alignAll();

    qDebug() << "[DxfView] lines ="     << m_model.lines.size()
             << "circles ="   << m_model.circles.size()
             << "polylines =" << m_model.polylines.size()
             << "texts ="     << m_model.texts.size();

    redrawFromModel();
    return true;
}

void DxfView::wheelEvent(QWheelEvent *event)
{
    const double factor = (event->angleDelta().y()>0) ? 1.15 : 1.0/1.15;
    scale(factor,factor);
}

void DxfView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    // 창 크기 바뀔 때도 다시 맞추기
    fitToContent();
}

void DxfView::redrawFromModel()
{
    if (!m_scene) {
        m_scene = new QGraphicsScene(this);
        setScene(m_scene);
    }

    // ---- 여기서 렌더러에게 전부 위임 ----
    m_renderer.render(m_scene, m_model);

    fitToContent();
}

void DxfView::fitToContent()
{
    if (!m_scene || m_scene->items().isEmpty())
        return;

    QRectF box = m_scene->itemsBoundingRect();
    if (box.isNull())
        return;

    // 약간 여백 주고 싶으면 marginsAdded 사용
    box = box.marginsAdded(QMarginsF(10,10,10,10));

    m_scene->setSceneRect(box);

    // ★ 이전 스케일 / 이동 모두 초기화
    resetTransform();

    // ★ 현재 뷰포트 크기에 맞게 최대한 키워서 보여줌
    fitInView(box, Qt::KeepAspectRatio);
}
