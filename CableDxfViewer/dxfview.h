#ifndef DXFVIEW_H
#define DXFVIEW_H

#pragma once

#include <QGraphicsView>
#include "dxfmodel.h"
#include "dxfrenderer.h"

class DxfView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DxfView(QWidget* parent=nullptr);

    bool loadDxfFile(const QString& filePath);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsScene* m_scene = nullptr;
    DxfModel        m_model;
    DxfRenderer     m_renderer;

    void redrawFromModel();
    void fitToContent();
};

#endif // DXFVIEW_H
