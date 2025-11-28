#ifndef DXFLOADER_H
#define DXFLOADER_H
#pragma once

#include "dxfmodel.h"
#include "dxfblockresolver.h"
#include "lineentity.h"
#include "polygonentity.h"
#include "textentity.h"

#include "dl_dxf.h"
#include "dl_creationadapter.h"
#include "dl_attributes.h"

class dxfloader : public DL_CreationAdapter
{
public:
    explicit dxfloader(DxfModel& model);

    static bool LoadFile(const QString& filePath, DxfModel& outModel, QString& outError);

    // --- 엔티티 콜백들 ---
    void addLine(const DL_LineData& data) override;
    void addPolyline(const DL_PolylineData& data) override;
    void addVertex(const DL_VertexData& data) override;
    void addCircle(const DL_CircleData& data) override;
    void addText(const DL_TextData& data) override;
    void addMText(const DL_MTextData& data) override;

    void addBlock(const DL_BlockData& data) override;
    void endBlock() override;
    void addInsert(const DL_InsertData& data) override;

private:
    DxfModel&         m_model;
    DxfBlockResolver  m_blockResolver;

    LineEntity    m_lineEntity;
    PolygonEntity m_polygonEntity;
    TextEntity    m_textEntity;

    DL_Attributes getAttributes() const;
};

#endif // DXFLOADER_H
