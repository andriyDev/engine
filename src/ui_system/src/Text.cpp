
#include "ui/Text.h"

#include "ui/UIUtil.h"

shared_ptr<MaterialProgram> Text::textProgram;
shared_ptr<Material> Text::textMaterial;

Text::Text()
{
    if(!textMaterial) {
        shared_ptr<Shader> common_shader = Shader::loadDirectly<Shader>("res/ui_common.s");
        shared_ptr<Shader> vertex_shader = Shader::loadDirectly<Shader>("res/ui_text.v");
        shared_ptr<Shader> fragment_shader = Shader::loadDirectly<Shader>("res/ui_text.f");

        if(!common_shader || !vertex_shader || !fragment_shader) {
            throw "Failed to load UI shaders.";
        }

        textProgram = make_shared<MaterialProgram>(vector<ResourceRef<Shader>>({
            common_shader, vertex_shader
        }), vector<ResourceRef<Shader>>({
            common_shader, fragment_shader
        }));

        textMaterial = make_shared<Material>(textProgram);
    }
}

void Text::render(vec4 mask, vec2 surfaceSize)
{
    shared_ptr<Font> fontPtr = font.resolve(Deferred);
    if(!fontPtr) {
        return;
    }
    vec4 rect = getLayoutBox();
    float newWidth = rect.z - rect.x;
    if(textNeedsUpdate || newWidth != layoutWidth) {
        layoutWidth = newWidth;
        textNeedsUpdate = false;
        if(useUnboundedLayout) {
            textLayout = textDesiredLayout;
            float shift = newWidth;
            if(textAlign == Font::Center) {
                shift *= 0.5f;
            }
            for(uint i = 0; i < textLayout.layout.size(); i++) {
                textLayout.layout[i].physicalLayout.x += shift;
                textLayout.layout[i].physicalLayout.z += shift;
            }
            for(uint i = 0; i < textLayout.advancePoints.size(); i++) {
                textLayout.advancePoints[i].x += shift;
                textLayout.prePoints[i].x += shift;
            }
            textLayout.bounds.x += shift;
            textLayout.bounds.z += shift;
        } else {
            textLayout = fontPtr->layoutString(text, fontSize, layoutWidth, textAlign, lineSpacing);
        }
    }

    if(textLayout.layout.size() > 0) {
        textMaterial->setTexture("font_texture", fontPtr->getTextureSheet());
        
        textMaterial->use();
        textMaterial->setVec2Property("surface_size", surfaceSize, true);
        textMaterial->setVec4Property("rect", rect, true);
        textMaterial->setVec4Property("mask", mask, true);
        textMaterial->setVec4Property("colour", colour, true);
        GLint uniformId = textMaterial->getUniformId("character_layout");
        glUniform4fv(uniformId, (GLsizei)textLayout.layout.size() * 2, (float*)&textLayout.layout[0]);
        UIUtil::bindRectangle();
        UIUtil::renderRectangles((uint)textLayout.layout.size());
    }
}

pair<UILayoutRequest, bool> Text::computeLayoutRequest()
{
    UILayoutRequest info;
    shared_ptr<Font> fontPtr = font.resolve(Deferred);
    if(!fontPtr) {
        info.desiredSize = vec2(0,0);
        return make_pair(info, true);
    }
    if(desiredNeedsUpdate) {
        desiredNeedsUpdate = false;
        textDesiredLayout = fontPtr->layoutStringUnbounded(text, fontSize, textAlign, lineSpacing);
    }
    info.desiredSize = getBoxSize(textDesiredLayout.bounds) + vec2(1,1);
    return make_pair(info, false);
}
