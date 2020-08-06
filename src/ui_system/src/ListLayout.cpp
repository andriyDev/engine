
#include "ui/ContainerLayouts.h"

#define isHorizontal(dir) (dir == ListDirection::Row || dir == ListDirection::RowReverse\
    || dir == ListDirection::RowCentered)

#define isReversed(dir) (dir == ListDirection::ColumnReverse || dir == ListDirection::RowReverse)

inline vec2 marginSize(const vec4& margin)
{
    return vec2(margin.x + margin.z, margin.y + margin.w);
}

template<ListDirection dir>
void configureCross(const shared_ptr<UIElement>& element, vec4& box, const vec4& rect, const vec2& desiredSize)
{
    const UIElement::LayoutDetails& eld = element->getLayoutDetails();
    if(isHorizontal(dir)) {
        if(eld.verticalGravity == UIElement::Start) {
            box.y = rect.y + eld.margin.y;
            box.w = box.y + desiredSize.y;
        } else if (eld.verticalGravity == UIElement::End) {
            box.w = rect.w - eld.margin.w;
            box.y = box.w - desiredSize.y;
        } else if (eld.verticalGravity == UIElement::Center) {
            float center = (rect.y + rect.w) * 0.5f;
            box.y = center - desiredSize.y * 0.5f;
            box.w = center + desiredSize.y * 0.5f;
        } else {
            box.y = rect.y + eld.margin.y;
            box.w = rect.w - eld.margin.w;
        }
    } else {
        if(eld.horizontalGravity == UIElement::Start) {
            box.x = rect.x + eld.margin.x;
            box.z = box.x + desiredSize.x;
        } else if (eld.horizontalGravity == UIElement::End) {
            box.z = rect.z - eld.margin.z;
            box.x = box.z - desiredSize.x;
        } else if (eld.horizontalGravity == UIElement::Center) {
            float center = (rect.x + rect.z) * 0.5f;
            box.x = center - desiredSize.x * 0.5f;
            box.z = center + desiredSize.x * 0.5f;
        } else {
            box.x = rect.x + eld.margin.x;
            box.z = rect.z - eld.margin.z;
        }
    }
}

template<ListDirection dir>
void maintainAspect(vec2& size, const vec4& box)
{
    if(isHorizontal(dir)) {
        size.x = size.y == 0 ? 0 : (size.x / size.y * glm::max(box.w - box.y, 0.f));
    } else {
        size.y = size.x == 0 ? 0 : (size.y / size.x * glm::max(box.z - box.x, 0.f));
    }
}

template<ListDirection dir>
void applySizing(vec2& desiredSize, const vec2& size)
{
    if(isHorizontal(dir)) {
        if(size.x != 0) {
            desiredSize.x = size.x;
        }
    } else {
        if(size.y != 0) {
            desiredSize.y = size.y;
        }
    }
}

template<ListDirection dir>
vector<vec4> ListLayout<dir>::layoutElements(const UIElement* rootElement, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements) const
{
    float weightSum = 0.f;
    float basisProduct = 0.f;
    float mainSize = 0;
    
    vector<vec4> boxes;
    boxes.reserve(elements.size());

    for(shared_ptr<UIElement> element : elements) {
        UILayoutRequest info = element->getLayoutRequest();
        vec4 box;
        vec2 size = info.desiredSize;
        
        configureCross<dir>(element, box, rect, size);

        if(info.maintainAspect) {
            maintainAspect<dir>(size, box);
        }

        const UIElement::LayoutDetails& eld = element->getLayoutDetails();
        applySizing<dir>(size, eld.size);
        // Since the box's coordinates along the main axis haven't been defined,
        // we can use them to store the desired size until they are defined.
        if(isHorizontal(dir)) {
            box.x = size.x;
        } else {
            box.y = size.y;
        }

        // This incomplete box is put into the list of boxes.
        boxes.push_back(box);

        weightSum += eld.weight;
        if(isHorizontal(dir)) {
            mainSize += size.x + eld.margin.x + eld.margin.z;
            basisProduct += size.x * eld.weight;
        } else {
            mainSize += size.y + eld.margin.y + eld.margin.w;
            basisProduct += size.y * eld.weight;
        }
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);
    float sizeDelta = (isHorizontal(dir) ? rect.z - rect.x : rect.w - rect.y) - mainSize;

    vector<float> sizesAlongMain;
    vector<bool> canBeResized;
    sizesAlongMain.resize(elements.size());
    canBeResized.resize(elements.size());
    for(uint i = 0; i < elements.size(); i++) {
        const shared_ptr<UIElement>& element = elements[i];
        sizesAlongMain[i] = isHorizontal(dir) ? boxes[i].x : boxes[i].y;
        canBeResized[i] = element->getLayoutDetails().weight != 0;
    }

    while(abs(sizeDelta) > FLT_EPSILON) {
        uint eventElement = (uint)elements.size();
        float eventDelta = INFINITY * sign(sizeDelta);
        for(uint i = 0; i < elements.size(); i++) {
            if(!canBeResized[i]) {
                continue;
            }
            const shared_ptr<UIElement>& element = elements[i];
            const UIElement::LayoutDetails& eld = element->getLayoutDetails();
            float sizeAlongMain = sizesAlongMain[i];
            float originalDesiredSize = isHorizontal(dir) ? boxes[i].x : boxes[i].y;
            if(sizeDelta > 0) {
                float maxSize = (isHorizontal(dir) ? eld.maxSize.x : eld.maxSize.y);
                float newDelta = (maxSize - sizeAlongMain) * weightSum / eld.weight;
                if(newDelta < eventDelta) {
                    eventElement = i;
                    eventDelta = newDelta;
                }
            } else {
                float minSize = (isHorizontal(dir) ? eld.minSize.x : eld.minSize.y);
                float newDelta = (minSize - sizeAlongMain) * basisProduct / (eld.weight * originalDesiredSize);
                if(newDelta > eventDelta) {
                    eventElement = i;
                    eventDelta = newDelta;
                }
            }
        }
        // If sizeDelta occurs before eventDelta. We just check whether sizeDelta is positive or negative and
        // whether it is before or after eventDelta respectively.
        if(sizeDelta > 0 != sizeDelta > eventDelta) {
            eventDelta = sizeDelta;
        }
        for(uint i = 0; i < elements.size(); i++) {
            if(!canBeResized[i]) {
                continue;
            }
            const shared_ptr<UIElement>& element = elements[i];
            const UIElement::LayoutDetails& eld = element->getLayoutDetails();
            float& sizeAlongMain = sizesAlongMain[i];
            float originalDesiredSize = isHorizontal(dir) ? boxes[i].x : boxes[i].y;
            if(sizeDelta > 0) {
                sizeAlongMain += eventDelta * eld.weight / weightSum;
            } else {
                sizeAlongMain += eventDelta * eld.weight * originalDesiredSize / basisProduct;
            }
        }
        sizeDelta -= eventDelta;
        if(eventElement == elements.size()) {
            break;
        }
        const UIElement::LayoutDetails& eeld = elements[eventElement]->getLayoutDetails();
        basisProduct -= (isHorizontal(dir) ? boxes[eventElement].x : boxes[eventElement].y)
            * eeld.weight;
        weightSum -= eeld.weight;
        canBeResized[eventElement] = false;
    }

    float offset = 0;
    for(uint i = 0; i < elements.size(); i++) {
        const shared_ptr<UIElement>& element = elements[i];
        const UIElement::LayoutDetails& eld = element->getLayoutDetails();
        vec4& box = boxes[i];
        float sizeAlongMain = sizesAlongMain[i];

        if(isHorizontal(dir)) {
            if(isReversed(dir)) {
                box.z = rect.z - offset - eld.margin.z;
                box.x = box.z - sizeAlongMain;
            } else {
                box.x = rect.x + offset + eld.margin.x;
                box.z = box.x + sizeAlongMain;
            }
            offset += sizeAlongMain + eld.margin.x + eld.margin.z;
        } else {
            if(isReversed(dir)) {
                box.w = rect.y - offset - eld.margin.w;
                box.y = box.w - sizeAlongMain;
            } else {
                box.y = rect.y + offset + eld.margin.y;
                box.w = box.y + sizeAlongMain;
            }
            offset += sizeAlongMain + eld.margin.y + eld.margin.w;
        }
        offset += spaceBetweenElements;
    }

    return boxes;
}

template<ListDirection dir>
UILayoutRequest ListLayout<dir>::computeLayoutRequest(const UIElement* rootElement,
    const vector<shared_ptr<UIElement>>& elements) const
{
    UILayoutRequest info;
    vec2& sum = info.desiredSize;
    sum = vec2(0,0);
    float maxCross = 0;
    {
        // We now compute the total space between elements, which is just one less than the elements times space.
        float spaceBetween = spaceBetweenElements * (elements.size() - 1);
        // Add that space to the correct direction.
        if(isHorizontal(dir)) {
            sum.x += spaceBetween;
        } else {
            sum.y += spaceBetween;
        }
    }
    for(int i = 0; i < elements.size(); i++) {
        vec2 size = elements[i]->getLayoutRequest().desiredSize;
        const UIElement::LayoutDetails& eld = elements[i]->getLayoutDetails();
        applySizing<dir>(size, eld.size);
        size += marginSize(eld.margin);
        // Now for each child, add its size along the main axis, take the max along the cross axis.
        if(isHorizontal(dir)) {
            sum.x += size.x;
            maxCross = glm::max(size.y, maxCross);
        } else {
            sum.y += size.y;
            maxCross = glm::max(size.x, maxCross);
        }
    }
    // Handle the cross direction.
    if(isHorizontal(dir)) {
        sum.y += maxCross;
    } else {
        sum.x += maxCross;
    }
    return info;
}

template class ListLayout<ListDirection::Row>;
template class ListLayout<ListDirection::RowReverse>;
template class ListLayout<ListDirection::Column>;
template class ListLayout<ListDirection::ColumnReverse>;

template<>
vector<vec4> ListLayout<ListDirection::RowCentered>::layoutElements(const UIElement* rootElement, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements) const
{
    vector<vec4> boxes;
    boxes.reserve(elements.size());
    float mainSize = 0;
    for(shared_ptr<UIElement> element : elements) {
        vec4 box;
        UILayoutRequest info = element->getLayoutRequest();

        configureCross<ListDirection::RowCentered>(element, box, rect, info.desiredSize);

        vec2 size = info.desiredSize;
        if(info.maintainAspect) {
            maintainAspect<ListDirection::RowCentered>(size, box);
        }
        const UIElement::LayoutDetails& eld = element->getLayoutDetails();
        applySizing<ListDirection::RowCentered>(size, eld.size);
        mainSize += size.x + eld.margin.x + eld.margin.z;
        boxes.push_back(box);
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);

    float offset = ((rect.x + rect.z) - mainSize) * 0.5f;
    for(uint i = 0; i < elements.size(); i++) {
        vec4& box = boxes[i];
        const shared_ptr<UIElement>& element = elements[i];
        UILayoutRequest info = element->getLayoutRequest();
        vec2 size = info.desiredSize;

        if(info.maintainAspect) {
            maintainAspect<ListDirection::ColumnCentered>(size, box);
        }
        const UIElement::LayoutDetails& eld = element->getLayoutDetails();
        applySizing<ListDirection::RowCentered>(size, eld.size);
        size.x += 1;
        
        box.x = offset + eld.margin.x;
        box.z = box.x + size.x;
        
        offset += size.x + eld.margin.x + eld.margin.z;
        offset += spaceBetweenElements;
    }

    return boxes;
}

template<>
vector<vec4> ListLayout<ListDirection::ColumnCentered>::layoutElements(const UIElement* rootElement, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements) const
{
    vector<vec4> boxes;
    boxes.reserve(elements.size());
    float mainSize = 0;
    for(const shared_ptr<UIElement>& element : elements) {
        vec4 box;
        UILayoutRequest info = element->getLayoutRequest();

        configureCross<ListDirection::ColumnCentered>(element, box, rect, info.desiredSize);
        
        vec2 size = info.desiredSize;
        if(info.maintainAspect) {
            maintainAspect<ListDirection::ColumnCentered>(size, box);
        }
        const UIElement::LayoutDetails& eld = element->getLayoutDetails();
        applySizing<ListDirection::ColumnCentered>(size, eld.size);
        mainSize += size.y + eld.margin.y + eld.margin.w;
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);

    float offset = ((rect.y + rect.w) - mainSize) * 0.5f;
    for(uint i = 0; i < elements.size(); i++) {
        vec4& box = boxes[i];
        const shared_ptr<UIElement>& element = elements[i];
        UILayoutRequest info = element->getLayoutRequest();
        vec2 size = info.desiredSize;

        if(info.maintainAspect) {
            maintainAspect<ListDirection::ColumnCentered>(size, box);
        }
        const UIElement::LayoutDetails& eld = element->getLayoutDetails();
        applySizing<ListDirection::ColumnCentered>(size, eld.size);
        size.y += 1;
        
        box.y = offset + eld.margin.y;
        box.w = box.y + size.y;
        
        offset += size.y + eld.margin.y + eld.margin.w;
        offset += spaceBetweenElements;

        boxes.push_back(box);
    }

    return boxes;
}

template class ListLayout<ListDirection::RowCentered>;
template class ListLayout<ListDirection::ColumnCentered>;
