
#include "ui/Image.h"
#include "UIUtil.h"

shared_ptr<MaterialProgram> Image::imageProgram;
shared_ptr<Material> Image::imageMaterial;

Image::Image()
{
    if(!imageMaterial) {
        shared_ptr<Shader> common_shader = Shader::loadDirectly<Shader>("res/ui_common.s");
        shared_ptr<Shader> vertex_shader = Shader::loadDirectly<Shader>("res/ui_image.v");
        shared_ptr<Shader> fragment_shader = Shader::loadDirectly<Shader>("res/ui_image.f");

        if(!common_shader || !vertex_shader || !fragment_shader) {
            throw "Failed to load UI shaders.";
        }

        imageProgram = make_shared<MaterialProgram>(vector<ResourceRef<Shader>>({
            common_shader, vertex_shader
        }), vector<ResourceRef<Shader>>({
            common_shader, fragment_shader
        }));

        if(!imageProgram) {
            throw "Failed to create UI program.";
        }

        imageMaterial = make_shared<Material>(imageProgram);

        if(!imageMaterial) {
            throw "Failed to create UI material.";
        }
    }
}

UILayoutInfo Image::layout(hash_map<const UIElement*, UILayoutInfo>& layoutInfo)
{
    UILayoutInfo info;
    shared_ptr<RenderableTexture> imagePtr = image.resolve(Deferred);
    if(imagePtr) {
        info.desiredSize = vec2(imagePtr->getWidth(), imagePtr->getHeight());
    } else {
        info.desiredSize = vec2(0,0);
    }
    layoutInfo.insert(make_pair(this, info));
    return info;
}

void Image::render(vec4 rect, vec4 mask, vec2 surfaceSize, const hash_map<const UIElement*, UILayoutInfo>& layoutInfo)
{
    imageMaterial->use();
    imageMaterial->setTexture("image", image, true);
    imageMaterial->setVec2Property("surface_size", surfaceSize, true);
    imageMaterial->setVec4Property("rect", rect, true);
    imageMaterial->setVec4Property("mask", mask, true);
    imageMaterial->setVec4Property("colour_tint", tint, true);
    imageMaterial->setVec4Property("corner_radii", cornerRadii, true);
    UIUtil::bindRectangle();
    UIUtil::renderRectangle();
}
