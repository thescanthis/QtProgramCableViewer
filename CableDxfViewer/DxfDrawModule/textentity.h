#ifndef TEXTENTITY_H
#define TEXTENTITY_H

#include "dxfmodel.h"

class DL_TextData;
class DL_MTextData;
class DL_Attributes;
class DxfBlockResolver;

/// TEXT / MTEXT 전담 헬퍼
class TextEntity
{
public:
    TextEntity(DxfModel& model, DxfBlockResolver& resolver);

    void addText(const DL_TextData& data, const DL_Attributes& attr);
    void addMText(const DL_MTextData& data, const DL_Attributes& attr);

private:
    DxfModel&         m_model;
    DxfBlockResolver& m_blockResolver;
};

#endif // TEXTENTITY_H
