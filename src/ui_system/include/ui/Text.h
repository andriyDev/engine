
#pragma once

#include "std.h"

#include "resources/ResourceLoader.h"
#include "font/Font.h"
#include "UIElement.h"
#include "renderer/Material.h"
#include "renderer/MaterialProgram.h"

#define GLEW_STATIC
#include <GL/glew.h>

class Text : public UIElement
{
public:
    ResourceRef<Font> font;

    Text();

    vec4 colour = vec4(1,1,1,1);

    void setText(const string& _text) {
        text = _text;
        textNeedsUpdate = true;
        desiredNeedsUpdate = true;
    }

    void setFontSize(float _size) {
        fontSize = _size;
        textNeedsUpdate = true;
        desiredNeedsUpdate = true;
    }

    void setLineSpacing(float _lineSpacing) {
        lineSpacing = _lineSpacing;
        textNeedsUpdate = true;
        desiredNeedsUpdate = true;
    }

    void setTextAlignment(Font::Alignment _alignment) {
        textAlign = _alignment;
        textNeedsUpdate = true;
    }

    void setUnboundedLayout(bool useUnbounded) {
        useUnboundedLayout = useUnbounded;
        textNeedsUpdate = true;
    }

    virtual void render(vec4 mask, vec2 surfaceSize) override;

    const Font::StringLayout& getTextLayout() const { return textLayout; }
protected:
    virtual pair<UILayoutRequest, bool> computeLayoutRequest() override;

    string text;
    float fontSize = 1.0f;
    float lineSpacing = 1.0f;
    float layoutWidth = 0;
    Font::Alignment textAlign = Font::Alignment::Left;
    bool useUnboundedLayout = false;
    bool textNeedsUpdate = true;
    bool desiredNeedsUpdate = true;

    Font::StringLayout textLayout;
    Font::StringLayout textDesiredLayout;
    
    static shared_ptr<MaterialProgram> textProgram;
    static shared_ptr<Material> textMaterial;
};
