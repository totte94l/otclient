#include "uibutton.h"
#include <framework/graphics/borderimage.h>
#include <framework/graphics/font.h>
#include <framework/otml/otmlnode.h>
#include <framework/luascript/luainterface.h>
#include <framework/graphics/graphics.h>

UIButton::UIButton(): UIWidget(UITypeButton)
{
    m_state = ButtonUp;
    m_focusable = false;

    // by default, all callbacks call lua fields
    m_onClick = [this]() { this->callLuaField("onClick"); };
}

UIButtonPtr UIButton::create()
{
    UIButtonPtr button(new UIButton);
    button->setStyle("Button");
    return button;
}

void UIButton::loadStyleFromOTML(const OTMLNodePtr& styleNode)
{
    UIWidget::loadStyleFromOTML(styleNode);

    for(int i=0; i<3; ++i) {
        m_statesStyle[i].image = m_image;
        m_statesStyle[i].color = m_backgroundColor;
        m_statesStyle[i].foregroundColor = m_foregroundColor;
        m_statesStyle[i].textTranslate = Point(0,0);
    }

    if(OTMLNodePtr node = styleNode->get("state.up"))
        loadStateStyle(m_statesStyle[ButtonUp], node);
    if(OTMLNodePtr node = styleNode->get("state.hover"))
        loadStateStyle(m_statesStyle[ButtonHover], node);
    if(OTMLNodePtr node = styleNode->get("state.down"))
        loadStateStyle(m_statesStyle[ButtonDown], node);

    m_text = styleNode->valueAt("text", fw::empty_string);

    if(OTMLNodePtr node = styleNode->get("onClick")) {
        g_lua.loadFunction(node->value(), "@" + node->source() + "[" + node->tag() + "]");
        luaSetField(node->tag());
    }
}

void UIButton::loadStateStyle(ButtonStateStyle& stateStyle, const OTMLNodePtr& stateStyleNode)
{
    if(OTMLNodePtr node = stateStyleNode->get("border-image"))
        stateStyle.image = BorderImage::loadFromOTML(node);
    if(OTMLNodePtr node = stateStyleNode->get("image"))
        stateStyle.image = Image::loadFromOTML(node);
    stateStyle.textTranslate = stateStyleNode->valueAt("text-translate", Point());
    stateStyle.color = stateStyleNode->valueAt("font-color", m_foregroundColor);
    stateStyle.color = stateStyleNode->valueAt("color", m_backgroundColor);
}

void UIButton::render()
{
    UIWidget::render();

    const ButtonStateStyle& currentStyle = m_statesStyle[m_state];
    Rect textRect = getRect();

    if(currentStyle.image) {
        g_graphics.bindColor(currentStyle.color);
        currentStyle.image->draw(textRect);
    }

    textRect.translate(currentStyle.textTranslate);
    m_font->renderText(m_text, textRect, AlignCenter, currentStyle.foregroundColor);
}

void UIButton::onHoverChange(UIHoverEvent& event)
{
    if(event.mouseEnter() && m_state == ButtonUp)
        m_state = ButtonHover;
    else if(m_state == ButtonHover)
        m_state = ButtonUp;
}

void UIButton::onMousePress(UIMouseEvent& event)
{
    if(event.button() == MouseLeftButton) {
        m_state = ButtonDown;
    } else
        event.ignore();
}

void UIButton::onMouseRelease(UIMouseEvent& event)
{
    if(m_state == ButtonDown) {
        if(m_onClick && getRect().contains(event.pos()))
            m_onClick();
        m_state = (isHovered() && isEnabled()) ? ButtonHover : ButtonUp;
    } else
        event.ignore();
}
