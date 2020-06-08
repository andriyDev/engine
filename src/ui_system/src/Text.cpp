
#include "ui/Text.h"

#include "UIUtil.h"

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

        if(!textProgram) {
            throw "Failed to create UI program.";
        }

        textMaterial = make_shared<Material>(textProgram);

        if(!textMaterial) {
            throw "Failed to create UI material.";
        }
    }
}

vec2 Text::layout(hash_map<const UIElement*, vec2>& desiredSizes) const
{
    desiredSizes.insert(make_pair(this, vec2(0,0)));
    return vec2(0,0);
}

void Text::render(vec4 rect, vec4 mask, vec2 surfaceSize, const hash_map<const UIElement*, vec2>& desiredSizes)
{
    shared_ptr<Font> fontPtr = font.resolve(Immediate);
    float newWidth = rect.z - rect.x;
    if(textNeedsUpdate || newWidth != layoutWidth) {
        layoutWidth = newWidth;
        textNeedsUpdate = false;
        textLayout = fontPtr->layoutString(text, 2, layoutWidth);
    }

    textMaterial->setTexture("font_texture", fontPtr->getTextureSheet());
    
    textMaterial->use();
    textMaterial->setVec2Property("surface_size", surfaceSize, true);
    textMaterial->setVec4Property("rect", rect, true);
    textMaterial->setVec4Property("mask", mask, true);
    GLint uniformId = textMaterial->getUniformId("character_layout");
    glUniform4fv(uniformId, (GLsizei)textLayout.size() * 2, (float*)&textLayout[0]);
    UIUtil::bindRectangle();
    UIUtil::renderRectangles((uint)textLayout.size());
}
