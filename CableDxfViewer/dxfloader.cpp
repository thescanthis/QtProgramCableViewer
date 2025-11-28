#include "dxfloader.h"


dxfloader::dxfloader(DxfModel &model)
    : m_model(model), m_blockResolver(), m_lineEntity(m_model, m_blockResolver), m_polygonEntity(m_model, m_blockResolver), m_textEntity(m_model, m_blockResolver)
{
}

bool dxfloader::LoadFile(const QString &filePath, DxfModel &outModel, QString &outError)
{
    outModel.clear();
    outError.clear();

    QFileInfo fi(filePath);
    if (!fi.exists() || !fi.isFile()) {
        outError = QStringLiteral("파일이 존재하지 않습니다: %1").arg(filePath);
        return false;
    }

    DL_Dxf dxf;
    dxfloader loader(outModel);

    const std::string path = filePath.toStdString();
    if (!dxf.in(path, &loader)) {
        outError = QStringLiteral("DXF 파싱 실패: %1").arg(filePath);
        return false;
    }

    if (outModel.empty()) {
        qWarning() << "DXF 로드 완료, 하지만 엔티티가 없습니다.";
    }

    return true;
}

void dxfloader::addLine(const DL_LineData &data)
{
    DL_Attributes attr = getAttributes();
    m_lineEntity.addLine(data, attr);
}

void dxfloader::addPolyline(const DL_PolylineData &data)
{
    DL_Attributes attr = getAttributes();
    m_lineEntity.addPolyline(data, attr);
}

void dxfloader::addVertex(const DL_VertexData &data)
{
    m_lineEntity.addVertex(data);
}

void dxfloader::addCircle(const DL_CircleData &data)
{
    DL_Attributes attr = getAttributes();
    m_polygonEntity.addCircle(data, attr);
}

void dxfloader::addText(const DL_TextData &data)
{
    DL_Attributes attr = getAttributes();
    m_textEntity.addText(data, attr);
}

void dxfloader::addMText(const DL_MTextData &data)
{
    DL_Attributes attr = getAttributes();
    m_textEntity.addMText(data, attr);
}

void dxfloader::addBlock(const DL_BlockData &data)
{
    m_blockResolver.beginBlock(data);
}

void dxfloader::endBlock()
{
    m_blockResolver.endBlock();
}

void dxfloader::addInsert(const DL_InsertData &data)
{
    m_blockResolver.addInsert(data, m_model);
}

DL_Attributes dxfloader::getAttributes() const
{
    // DL_CreationAdapter 가 제공하는 attributes 멤버를 그대로 반환
    return attributes;
}
