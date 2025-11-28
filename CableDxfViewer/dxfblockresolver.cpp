#include "dxfblockresolver.h"

void DxfBlockResolver::beginBlock(const DL_BlockData &data)
{
    QString name = QString::fromStdString(data.name);

    DxfBlockDef& blk = m_blocks[name];
    blk.name = name;
    blk.base = { data.bpx, data.bpy };

    blk.lines.clear();
    blk.circles.clear();
    blk.polylines.clear();
    blk.texts.clear();

    m_currentBlock = &blk;
    m_currentPolylineIndex = -1;

    qDebug() << "[BLOCK] begin" << name
             << "base =" << blk.base.x << blk.base.y;
}

void DxfBlockResolver::endBlock()
{
    if (m_currentBlock) {
        qDebug() << "[BLOCK] end" << m_currentBlock->name;
    }
    m_currentBlock = nullptr;
    m_currentPolylineIndex = -1;
}

void DxfBlockResolver::addLine(const DL_LineData &data, const DL_Attributes &attr)
{
    if (!m_currentBlock) return;

    DxfLine L;
    L.p1   = { data.x1, data.y1 };
    L.p2   = { data.x2, data.y2 };
    L.layer = QString::fromStdString(attr.getLayer());

    m_currentBlock->lines.push_back(L);
}

void DxfBlockResolver::addCircle(const DL_CircleData &data, const DL_Attributes &attr)
{
    if (!m_currentBlock) return;

    DxfCircle C;
    C.center = { data.cx, data.cy };
    C.radius = data.radius;
    C.layer  = QString::fromStdString(attr.getLayer());

    m_currentBlock->circles.push_back(C);
}

void DxfBlockResolver::addPolyline(const DL_PolylineData &data, const DL_Attributes &attr)
{
    if (!m_currentBlock) return;

    DxfPolyline P;
    P.closed = (data.flags & 1);
    P.layer  = QString::fromStdString(attr.getLayer());

    m_currentBlock->polylines.push_back(std::move(P));
    m_currentPolylineIndex =
        static_cast<int>(m_currentBlock->polylines.size()) - 1;
}

void DxfBlockResolver::addVertex(const DL_VertexData &data)
{
    if (!m_currentBlock) return;

    if (m_currentPolylineIndex < 0 ||
        m_currentPolylineIndex >= static_cast<int>(m_currentBlock->polylines.size()))
        return;

    DxfPoint pt{ data.x, data.y };
    m_currentBlock->polylines[m_currentPolylineIndex].points.push_back(pt);
}

void DxfBlockResolver::addText(const DL_TextData &data, const DL_Attributes &attr)
{
    if (!m_currentBlock) return;

    DxfText T;
    T.pos    = { data.apx, data.apy };
    T.text   = QString::fromStdString(data.text);
    T.height = data.height;
    T.layer  = QString::fromStdString(attr.getLayer());

    m_currentBlock->texts.push_back(T);
}

void DxfBlockResolver::addMText(const DL_MTextData &data, const DL_Attributes &attr)
{
    if (!m_currentBlock) return;

    DxfText T;
    // 필요하면 data.apx, data.apy 사용해서 alignment point 로 바꿀 수 있음
    T.pos    = { data.dirx, data.diry };
    T.text   = QString::fromStdString(data.text);
    T.height = data.height;
    T.layer  = QString::fromStdString(attr.getLayer());

    m_currentBlock->texts.push_back(T);

    qDebug() << "[MTEXT local]" << T.layer << T.text
             << "at" << T.pos.x << T.pos.y << "h=" << T.height;
}

void DxfBlockResolver::addInsert(const DL_InsertData &data, DxfModel &outModel)
{
    QString name = QString::fromStdString(data.name);
    auto it = m_blocks.find(name);
    if (it == m_blocks.end()) {
        qWarning() << "[INSERT] unknown block:" << name;
        return;
    }

    const DxfBlockDef& blk = it->second;

    int cols = data.cols > 0 ? data.cols : 1;
    int rows = data.rows > 0 ? data.rows : 1;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            DL_InsertData ins = data;
            ins.ipx = data.ipx + c * data.colSp;
            ins.ipy = data.ipy + r * data.rowSp;

            flattenBlockInstance(blk, ins, outModel);
        }
    }
}

DxfPoint DxfBlockResolver::transformPoint(const DxfPoint &p, const DxfBlockDef &blk, const DL_InsertData &ins) const
{
    double x = p.x - blk.base.x;
    double y = p.y - blk.base.y;

    x *= ins.sx;
    y *= ins.sy;

    double angleRad = ins.angle * M_PI / 180.0;

    double rx =  std::cos(angleRad) * x - std::sin(angleRad) * y;
    double ry =  std::sin(angleRad) * x + std::cos(angleRad) * y;

    rx += ins.ipx;
    ry += ins.ipy;

    return { rx, ry };
}

void DxfBlockResolver::flattenBlockInstance(const DxfBlockDef &blk, const DL_InsertData &ins, DxfModel &outModel)
{
    // LINE
    for (const auto& L : blk.lines) {
        DxfLine out;
        out.p1    = transformPoint(L.p1, blk, ins);
        out.p2    = transformPoint(L.p2, blk, ins);
        out.layer = L.layer;
        outModel.lines.push_back(out);
    }

    // CIRCLE
    for (const auto& C : blk.circles) {
        DxfCircle out;
        out.center = transformPoint(C.center, blk, ins);
        out.radius = C.radius * ins.sx;
        out.layer  = C.layer;
        outModel.circles.push_back(out);
    }

    // POLYLINE
    for (const auto& P : blk.polylines) {
        DxfPolyline out;
        out.closed = P.closed;
        out.layer  = P.layer;
        out.points.reserve(P.points.size());
        for (const auto& pt : P.points)
            out.points.push_back(transformPoint(pt, blk, ins));
        outModel.polylines.push_back(std::move(out));
    }

    // TEXT / MTEXT
    for (const auto& T : blk.texts) {
        DxfText out = T;
        out.pos    = transformPoint(T.pos, blk, ins);
        out.height = T.height * ins.sx;
        outModel.texts.push_back(out);

        qDebug() << "[MTEXT world]" << out.layer << out.text
                 << "at" << out.pos.x << out.pos.y
                 << "h=" << out.height;
    }

}
