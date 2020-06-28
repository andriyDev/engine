
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
        size = _size;
        textNeedsUpdate = true;
        desiredNeedsUpdate = true;
    }

    void setLineSpacing(float _lineSpacing) {
        lineSpacing = _lineSpacing;
        textNeedsUpdate = true;
        desiredNeedsUpdate = true;
    }

    virtual UILayoutInfo layout(hash_map<const UIElement*, UILayoutInfo>& layoutInfo) override;
    virtual void render(vec4 rect, vec4 mask, vec2 surfaceSize,
        const hash_map<const UIElement*, UILayoutInfo>& layoutInfo) override;
protected:
    string text;
    float size = 1.0f;
    float lineSpacing = 1.0f;
    float layoutWidth = 0;
    bool textNeedsUpdate = true;
    bool desiredNeedsUpdate = true;

    Font::StringLayout textLayout;
    Font::StringLayout textDesiredLayout;
    
    static shared_ptr<MaterialProgram> textProgram;
    static shared_ptr<Material> textMaterial;
};
