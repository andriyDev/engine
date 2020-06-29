
#pragma once

#include "std.h"

#include "UIElement.h"
#include "renderer/Material.h"

class Image : public UIElement
{
public:
    Image();

    ResourceRef<RenderableTexture> image;

    vec4 tint = vec4(1,1,1,1);
    // The radius of each corner in order of TL TR BL BR
    vec4 cornerRadii = vec4(0,0,0,0);

    virtual void render(vec4 mask, vec2 surfaceSize) override;
protected:
    virtual pair<UILayoutRequest, bool> computeLayoutRequest() override;

    static shared_ptr<MaterialProgram> imageProgram;
    static shared_ptr<Material> imageMaterial;
};
