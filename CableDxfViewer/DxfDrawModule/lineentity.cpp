#include "pch.h"
#include "lineentity.h"
#include "dxfblockresolver.h"
#include "dl_entities.h"
#include "dl_attributes.h"

LineEntity::LineEntity(DxfModel &model, DxfBlockResolver &resolver)
    : m_model(model)
    , m_blockResolver(resolver)
{
}

// -------------------------
// LINE 처리
// -------------------------
void LineEntity::addLine(const DL_LineData &data,
                         const DL_Attributes &attr)
{
    // BLOCK 안에 있으면 BlockResolver에 맡김
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addLine(data, attr);
        return;
    }

    // ---- 전역 LINE ----
    DxfLine L;
    L.p1   = { data.x1, data.y1 };
    L.p2   = { data.x2, data.y2 };
    L.layer = QString::fromStdString(attr.getLayer());

    m_model.lines.push_back(L);
}

// -------------------------
// POLYLINE 시작
// -------------------------
void LineEntity::addPolyline(const DL_PolylineData &data,
                             const DL_Attributes &attr)
{
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addPolyline(data, attr);
        return;
    }

    // ---- 전역 POLYLINE ----
    DxfPolyline P;
    P.closed = (data.flags & 1);
    P.layer  = QString::fromStdString(attr.getLayer());

    m_model.polylines.push_back(std::move(P));
    m_currentPolylineIndex =
        static_cast<int>(m_model.polylines.size()) - 1;
}

// -------------------------
// POLYLINE의 각 Vertex 추가
// -------------------------
void LineEntity::addVertex(const DL_VertexData &data)
{
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addVertex(data);
        return;
    }

    if (m_currentPolylineIndex >= 0) {
        auto& poly = m_model.polylines[m_currentPolylineIndex];
        poly.points.push_back({ data.x, data.y });

        // 👇 정점이 3개 이상일 때 암묵적 closed 판단
        if (!poly.closed && poly.points.size() >= 3) {
            const DxfPoint& first = poly.points.front();
            const DxfPoint& last  = poly.points.back();

            if (qFuzzyCompare(first.x, last.x) &&
                qFuzzyCompare(first.y, last.y)) {
                poly.closed = true;
            }
        }
    }
}

void LineEntity::buildConnectorGroups(DxfModel &model)
{
    // 1) 먼저 모든 단자(Terminal)를 추출해 둡니다.
    std::vector<TerminalItem> allTerminals = extractTerminals(model);
    std::vector<ConnectorGroup> groups;
    std::vector<ConnectorGroup> texts;

    // 2) 모든 텍스트를 순회하며 "이 텍스트가 그룹 이름인가?" 확인
    for (const auto& text : model.texts) {

        // 이미 단자로 쓰인 텍스트는 그룹 이름이 될 수 없으므로 스킵
        bool isTerminalLabel = false;
        for(const auto& t : allTerminals) {
            // 텍스트 내용과 위치가 같으면 단자 라벨임
            if(t.label.text == text.text &&
                qFuzzyCompare(t.label.pos.x, text.pos.x) &&
                qFuzzyCompare(t.label.pos.y, text.pos.y)) {
                isTerminalLabel = true;
                break;
            }
        }
        if (isTerminalLabel) continue;


        // 3) 이 텍스트(P1 등)와 겹치는 '큰 폴리라인' 찾기
        for (const auto& poly : model.polylines) {

            // 점 3개 미만은 선이 아니므로 패스
            if (poly.points.size() < 3) continue;

            // 폴리라인 형상 생성
            QPainterPath polyPath;
            polyPath.moveTo(poly.points[0].x, poly.points[0].y);
            for (size_t i = 1; i < poly.points.size(); ++i)
                polyPath.lineTo(poly.points[i].x, poly.points[i].y);

            // ★ 열린 'ㄷ'자 형태라도 내부 포함 여부를 알기 위해 닫아줍니다.
            polyPath.closeSubpath();

            // 텍스트 위치
            QPointF textPos(text.pos.x, text.pos.y);

            // ★ 핵심: 텍스트가 이 폴리라인 영역에 '겹쳐' 있거나 '포함'되어 있는지 확인
            // (contains가 아니더라도 intersects 등 상황에 맞는 검사 가능)
            if (polyPath.contains(textPos) || polyPath.intersects(QRectF(textPos.x()-1, textPos.y()-1, 2, 2))) {

                // ==> 매칭 성공! (텍스트: P1, 폴리라인: 그룹선)

                ConnectorGroup group;
                group.groupName = text;   // 그룹 이름 (P1)
                group.groupShape = poly;  // 그룹 모양 (Bracket)
                group.boundRect = polyPath.boundingRect();
                texts.push_back(group);

                // 4) 이제 이 '그룹선' 안에 들어있는 '단자들'을 싹 모읍니다.
                for (const auto& term : allTerminals) {
                    QPointF termCenter(term.label.pos.x, term.label.pos.y);

                    // 단자가 그룹 영역 안에 있는지 확인
                    if (polyPath.contains(termCenter)) {
                        group.terminals.push_back(term);
                    }
                }

                // 단자가 하나라도 있으면 유효한 그룹으로 추가
                if (!group.terminals.empty()) {
                    groups.push_back(group);
                    // 이 텍스트는 그룹 이름으로 판명났으니, 다른 폴리라인과는 매칭 안 함
                    break;
                }
            }
        }
    }

    // 결과 저장
    model.connectorGroups = groups;

    // 디버깅용 출력
    qDebug() << "========================================";
    qDebug() << "[Result] Total Groups Found:" << groups.size();
    for(const auto& g : groups) {
        QString termNames;
        for(const auto& t : g.terminals) termNames += t.label.text + ", ";
        qDebug() << " Group:" << g.groupName.text << " -> Terminals [" << termNames << "]";
    }
    qDebug() << "========================================";
}

std::vector<TerminalItem> LineEntity::extractTerminals(const DxfModel &model)
{
    std::vector<TerminalItem> terminals;

    for (const auto& poly : model.polylines) {
        // 단자는 닫힌 사각형이므로 점이 4~5개 정도여야 함
        if (poly.points.size() < 4) continue;

        // 닫혀있지 않더라도 시작점과 끝점이 같으면 닫힌걸로 간주
        bool isClosed = poly.closed;
        if (!isClosed && !poly.points.empty()) {
            const auto& p1 = poly.points.front();
            const auto& p2 = poly.points.back();
            if (qFuzzyCompare(p1.x, p2.x) && qFuzzyCompare(p1.y, p2.y))
                isClosed = true;
        }
        // 닫히지 않은 긴 선(그룹 선)은 제외
        if (!isClosed) continue;

        // 형상 생성
        QPainterPath path;
        path.moveTo(poly.points[0].x, poly.points[0].y);
        for (size_t i = 1; i < poly.points.size(); ++i)
            path.lineTo(poly.points[i].x, poly.points[i].y);
        path.closeSubpath();

        QRectF bounds = path.boundingRect();

        // 크기 필터링 (너무 큰 도형은 단자가 아님. 예: 전체 프레임)
        // 적절한 임계값은 실제 도면 스케일에 따라 조정 필요 (예: 100x100 이하)
        if (bounds.width() > 200 || bounds.height() > 200) continue;

        // 내부에 포함된 텍스트 찾기
        for (const auto& text : model.texts) {
            QPointF pt(text.pos.x, text.pos.y);

            // 박스 검사 후 정밀 검사
            if (bounds.contains(pt)) {
                if (path.contains(pt)) {
                    TerminalItem item;
                    item.shape = poly;
                    item.label = text;
                    terminals.push_back(item);
                    break; // 하나의 사각형엔 하나의 텍스트만 있다고 가정
                }
            }
        }
    }
    return terminals;
}
