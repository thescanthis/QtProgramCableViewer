#include "pch.h"

#include "textentity.h"
#include "dxfblockresolver.h"
#include "dl_entities.h"
#include "dl_attributes.h"

TextEntity::TextEntity(DxfModel &model, DxfBlockResolver &resolver)
    : m_model(model)
    , m_blockResolver(resolver)
{
}

// -------------------------
// TEXT
// -------------------------
void TextEntity::addText(const DL_TextData &data,
                         const DL_Attributes &attr)
{
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addText(data, attr);
        return;
    }

    DxfText T;
    // TEXT는 보통 ipx, ipy 를 기준 위치로 많이 씀
    T.pos    = { data.ipx, data.ipy };
    T.text   = QString::fromStdString(data.text);
    T.height = data.height;
    T.layer  = QString::fromStdString(attr.getLayer());

    m_model.texts.push_back(T);
}

// -------------------------
// MTEXT
// -------------------------
void TextEntity::addMText(const DL_MTextData &data,
                          const DL_Attributes &attr)
{
    if (m_blockResolver.isInBlock()) {
        m_blockResolver.addMText(data, attr);
        return;
    }

    DxfText T;
    // 네 도면 기준으로는 dirx, diry 가 실제 화면 위치에 더 가까웠음
    T.pos    = { data.dirx, data.diry };
    T.text   = QString::fromStdString(data.text);
    T.height = data.height;
    T.layer  = QString::fromStdString(attr.getLayer());

    m_model.texts.push_back(T);

    qDebug() << "[MTEXT global]" << T.layer << T.text
             << "at" << T.pos.x << T.pos.y << "h=" << T.height;
}
