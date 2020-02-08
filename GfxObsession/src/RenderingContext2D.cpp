#include <Gfx/RenderingContext.hpp>

#include <framework/utility/essentials.hpp>

namespace Obs::Gfx {

RenderingContext2D RenderingContext2D::WithAlignment(Alignment alignment) const {
    RenderingContext2D ctx {*this};
    ctx.alignment = alignment;
    return ctx;
}

RenderingContext2D RenderingContext2D::WithFont(RenderingKit::IFontFace& font) const {
    RenderingContext2D ctx {*this};
    ctx.font = &font;
    return ctx;
}

RenderingContext2D RenderingContext2D::WithTextColor(zfw::Byte4 color) const {
    RenderingContext2D ctx {*this};
    ctx.color = color;
    return ctx;
}

RenderingContext2D RenderingContext2D::WithPosLeftTop(glm::ivec2 pos) const {
    RenderingContext2D ctx {*this};
    ctx.pos = pos;
    return ctx;
}

RenderingContext2D RenderingContext2D::DrawTextRow(u8string_view text) const {
    zombie_assert(font != nullptr);

    zombie_assert(this->alignment == leftTop);
    int alignment = zfw::ALIGN_LEFT | zfw::ALIGN_TOP;

    font->DrawText(text, color, alignment, {pos.x, pos.y, 0}, {0, 0});

    RenderingContext2D ctx {*this};
    ctx.pos.y += font->MeasureText(text).y;
    return ctx;
}

}
