#ifndef DXFBLOCKRESOLVER_H
#define DXFBLOCKRESOLVER_H

#pragma once
#include "dxfmodel.h"
#include "dl_dxf.h"
#include "dl_attributes.h"

struct DxfBlockDef
{
    QString  name;
    DxfPoint base;   // block base point (bpx, bpy)

    std::vector<DxfLine>     lines;
    std::vector<DxfCircle>   circles;
    std::vector<DxfPolyline> polylines;
    std::vector<DxfText>     texts;
};

class DxfBlockResolver
{
public:
    DxfBlockResolver() = default;

    // --- BLOCK 섹션 ---
    void beginBlock(const DL_BlockData& data);
    void endBlock();

    bool isInBlock() const { return m_currentBlock != nullptr; }

    // BLOCK 안에 엔티티 추가 (속성 포함)
    void addLine(const DL_LineData& data, const DL_Attributes& attr);
    void addCircle(const DL_CircleData& data, const DL_Attributes& attr);
    void addPolyline(const DL_PolylineData& data, const DL_Attributes& attr);
    void addVertex(const DL_VertexData& data);
    void addText(const DL_TextData& data, const DL_Attributes& attr);
    void addMText(const DL_MTextData& data, const DL_Attributes& attr);

    // --- INSERT: BLOCK 을 월드좌표로 펼쳐서 outModel 에 추가 ---
    void addInsert(const DL_InsertData& data, DxfModel& outModel);

private:
    std::unordered_map<QString, DxfBlockDef> m_blocks;
    DxfBlockDef* m_currentBlock = nullptr;
    int          m_currentPolylineIndex = -1;  // 블록 안에서의 현재 폴리라인

    // 헬퍼들
    DxfPoint transformPoint(const DxfPoint& p, const DxfBlockDef& blk, const DL_InsertData& ins) const;

    void flattenBlockInstance(const DxfBlockDef& blk, const DL_InsertData& ins, DxfModel& outModel);
};

#endif // DXFBLOCKRESOLVER_H
