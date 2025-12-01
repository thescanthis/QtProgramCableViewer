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
    if (qFuzzyCompare(data.ipx, 1.0) && qFuzzyCompare(data.ipy, 0.0)
        && data.dirx > 100 && data.diry > 100) {
        T.pos = { data.dirx, data.diry }; // fallback 좌표
    } else {
        T.pos = { data.ipx, data.ipy };   // 일반적인 좌표
    }

    T.text   = QString::fromStdString(data.text);
    T.height = data.height;
    T.layer  = QString::fromStdString(attr.getLayer());

    m_model.texts.push_back(T);

    qDebug() << "[MTEXT global]" << T.layer << T.text
             << "at" << T.pos.x << T.pos.y << "h=" << T.height;
}
