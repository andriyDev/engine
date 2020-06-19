
#include "ui/Box.h"
#include "resources/Shader.h"

#include "UIUtil.h"

shared_ptr<MaterialProgram> Box::boxProgram;
shared_ptr<Material> Box::boxMaterial;

Box::Box()
{
    if(!boxMaterial) {
        shared_ptr<Shader> common_shader = Shader::loadDirectly<Shader>("res/ui_common.s");
        shared_ptr<Shader> vertex_shader = Shader::loadDirectly<Shader>("res/ui_generic.v");
        shared_ptr<Shader> fragment_shader = Shader::loadDirectly<Shader>("res/ui_generic.f");

        if(!common_shader || !vertex_shader || !fragment_shader) {
            throw "Failed to load UI shaders.";
        }

        boxProgram = make_shared<MaterialProgram>(vector<ResourceRef<Shader>>({
            common_shader, vertex_shader
        }), vector<ResourceRef<Shader>>({
            common_shader, fragment_shader
        }));

        if(!boxProgram) {
            throw "Failed to create UI program.";
        }

        boxMaterial = make_shared<Material>(boxProgram);

        if(!boxMaterial) {
            throw "Failed to create UI material.";
        }
    }
}

void Box::renderSelf(vec4 rect, vec4 mask, vec2 surfaceSize)
{
    boxMaterial->use();
    boxMaterial->setVec2Property("surface_size", surfaceSize, true);
    boxMaterial->setVec4Property("rect", rect, true);
    boxMaterial->setVec4Property("mask", mask, true);
    boxMaterial->setVec4Property("colour_tint", colour, true);
    UIUtil::bindRectangle();
    UIUtil::renderRectangle();
}
