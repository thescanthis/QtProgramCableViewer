#ifndef LINEENTITY_H
#define LINEENTITY_H
#pragma once

#include "dxfmodel.h"

// dxflib 타입 전방 선언 (헤더 의존 최소화)
class DL_LineData;
class DL_PolylineData;
class DL_VertexData;
class DL_Attributes;
class DxfBlockResolver;

/// - LINE / POLYLINE 전용 헬퍼
/// - dxfloader에서 콜백을 받아 대신 호출해 줌

class LineEntity
{
public:
    LineEntity(DxfModel& model, DxfBlockResolver& resolver);

    // LINE 콜백 처리
    void addLine(const DL_LineData& data, const DL_Attributes& attr);

    // POLYLINE 시작
    void addPolyline(const DL_PolylineData& data, const DL_Attributes& attr);
    static void GroupPolyLineCheck(DxfModel& model);

    // VERTEX (Polyline의 점 추가)
    void addVertex(const DL_VertexData& data);

private:
    DxfModel&        m_model;
    DxfBlockResolver& m_blockResolver;

    int m_currentPolylineIndex = -1;   // 현재 작성 중인 전역 polyline 인덱스
};

#endif // LINEENTITY_H
