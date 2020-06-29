
#include "ui/ContainerLayouts.h"

#define isHorizontal(dir) (dir == ListDirection::Row || dir == ListDirection::RowReverse\
    || dir == ListDirection::RowCentered)

#define isReversed(dir) (dir == ListDirection::ColumnReverse || dir == ListDirection::RowReverse)

inline vec2 marginSize(const vec4& margin)
{
    return vec2(margin.x + margin.z, margin.y + margin.w);
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
        // Use gravity to figure out the cross direction.
        if(isHorizontal(dir)) {
            if(element->verticalGravity == UIElement::Start) {
                box.y = rect.y + element->margin.y;
                box.w = box.y + size.y;
            } else if (element->verticalGravity == UIElement::End) {
                box.w = rect.w - element->margin.w;
                box.y = box.w - size.y;
            } else if (element->verticalGravity == UIElement::Center) {
                float center = (rect.y + rect.w) * 0.5f;
                box.y = center - size.y * 0.5f;
                box.w = center + size.y * 0.5f;
            } else {
                box.y = rect.y + element->margin.y;
                box.w = rect.w - element->margin.w;
            }
        } else {
            if(element->horizontalGravity == UIElement::Start) {
                box.x = rect.x + element->margin.x;
                box.z = box.x + size.x;
            } else if (element->horizontalGravity == UIElement::End) {
                box.z = rect.z - element->margin.z;
                box.x = box.z - size.x;
            } else if (element->horizontalGravity == UIElement::Center) {
                float center = (rect.x + rect.z) * 0.5f;
                box.x = center - size.x * 0.5f;
                box.z = center + size.x * 0.5f;
            } else {
                box.x = rect.x + element->margin.x;
                box.z = rect.z - element->margin.z;
            }
        }
        // This incomplete box is put into the list of boxes.
        boxes.push_back(box);

        if(info.maintainAspect) {
            if(isHorizontal(dir)) {
                size.x = size.y == 0 ? 0 : (size.x / size.y * max(box.w - box.y, 0.f));
            } else {
                size.y = size.x == 0 ? 0 : (size.y / size.x * max(box.z - box.x, 0.f));
            }
        }

        weightSum += element->weight;
        if(isHorizontal(dir)) {
            mainSize += size.x + element->margin.x + element->margin.z;
            basisProduct += size.x * element->weight;
        } else {
            mainSize += size.y + element->margin.y + element->margin.w;
            basisProduct += size.y * element->weight;
        }
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);
    float sizeDelta = (isHorizontal(dir) ? rect.z - rect.x : rect.w - rect.y) - mainSize;

    float offset = 0;
    for(uint i = 0; i < elements.size(); i++) {
        const shared_ptr<UIElement>& element = elements[i];
        vec4& box = boxes[i];
        UILayoutRequest info = element->getLayoutRequest();
        vec2 size = info.desiredSize;

        if(info.maintainAspect) {
            if(isHorizontal(dir)) {
                size.x = size.y == 0 ? 0 : (size.x / size.y * max(box.w - box.y, 0.f));
            } else {
                size.y = size.x == 0 ? 0 : (size.y / size.x * max(box.z - box.x, 0.f));
            }
        }
        // If the element has no weight, or the size delta is exactly 0,
        // just layout the element according to its desired size.
        float elementSizeDelta;
        if(element->weight == 0 || sizeDelta == 0) {
            elementSizeDelta = 0;
        } else if(sizeDelta > 0) {
            // Otherwise, grow or shrink the element based on the flexbox algorithm.
            elementSizeDelta = sizeDelta * element->weight / weightSum;
        } else {
            elementSizeDelta = sizeDelta * element->weight * (isHorizontal(dir) ? size.x : size.y) / basisProduct;
        }
        if(isHorizontal(dir)) {
            size.x += elementSizeDelta + 1;
            if(isReversed(dir)) {
                box.z = rect.z - offset - element->margin.z;
                box.x = box.z - size.x;
            } else {
                box.x = rect.x + offset + element->margin.x;
                box.z = box.x + size.x;
            }
            offset += size.x + element->margin.x + element->margin.z;
        } else {
            size.y += elementSizeDelta + 1;
            if(isReversed(dir)) {
                box.w = rect.y - offset - element->margin.w;
                box.y = box.w - size.y;
            } else {
                box.y = rect.y + offset + element->margin.y;
                box.w = box.y + size.y;
            }
            offset += size.y + element->margin.y + element->margin.w;
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
        vec2 size = elements[i]->getLayoutRequest().desiredSize + marginSize(elements[i]->margin);
        // Now for each child, add its size along the main axis, take the max along the cross axis.
        if(isHorizontal(dir)) {
            sum.x += size.x;
            maxCross = max(size.y, maxCross);
        } else {
            sum.y += size.y;
            maxCross = max(size.x, maxCross);
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

        if(element->verticalGravity == UIElement::Start) {
            box.y = rect.y + element->margin.y;
            box.w = box.y + info.desiredSize.y;
        } else if (element->verticalGravity == UIElement::End) {
            box.w = rect.w - element->margin.w;
            box.y = box.w - info.desiredSize.y;
        } else if (element->verticalGravity == UIElement::Center) {
            float center = (rect.y + rect.w) * 0.5f;
            box.y = center - info.desiredSize.y * 0.5f;
            box.w = center + info.desiredSize.y * 0.5f;
        } else {
            box.y = rect.y + element->margin.y;
            box.w = rect.w - element->margin.w;
        }

        vec2 size = info.desiredSize;
        if(info.maintainAspect) {
            size.x = size.y == 0 ? 0 : (size.x / size.y * (box.w - box.y));
        }
        mainSize += size.x + element->margin.x + element->margin.z;
        boxes.push_back(box);
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);
    float start_point = ((rect.x + rect.z) - mainSize) * 0.5f;

    float offset = 0;
    for(uint i = 0; i < elements.size(); i++) {
        vec4& box = boxes[i];
        const shared_ptr<UIElement>& element = elements[i];
        UILayoutRequest info = element->getLayoutRequest();
        vec2 size = info.desiredSize;

        if(info.maintainAspect) {
            size.x = size.y == 0 ? 0 : (size.x / size.y * max(box.w - box.y, 0.f));
        }
        size.x += 1;
        
        box.x = start_point + offset + element->margin.x;
        box.z = box.x + size.x;
        
        offset += size.x + element->margin.x + element->margin.z;
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
    for(shared_ptr<UIElement> element : elements) {
        vec4 box;
        UILayoutRequest info = element->getLayoutRequest();

        if(element->horizontalGravity == UIElement::Start) {
            box.x = rect.x + element->margin.x;
            box.z = box.x + info.desiredSize.x;
        } else if (element->horizontalGravity == UIElement::End) {
            box.z = rect.z - element->margin.z;
            box.x = box.z - info.desiredSize.x;
        } else if (element->horizontalGravity == UIElement::Center) {
            float center = (rect.x + rect.z) * 0.5f;
            box.x = center - info.desiredSize.x * 0.5f;
            box.z = center + info.desiredSize.x * 0.5f;
        } else {
            box.x = rect.x + element->margin.x;
            box.z = rect.z - element->margin.z;
        }
        
        vec2 size = info.desiredSize;
        if(info.maintainAspect) {
            size.y = size.x == 0 ? 0 : (size.y / size.x * max(box.z - box.x, 0.f));
        }
        mainSize += size.y + element->margin.y + element->margin.w;
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);
    float start_point = ((rect.y + rect.w) - mainSize) * 0.5f;

    float offset = 0;
    for(uint i = 0; i < elements.size(); i++) {
        vec4& box = boxes[i];
        const shared_ptr<UIElement>& element = elements[i];
        UILayoutRequest info = element->getLayoutRequest();
        vec2 size = info.desiredSize;
        info.desiredSize.y += 1;

        if(info.maintainAspect) {
            size.y = size.x == 0 ? 0 : (size.y / size.x * max(box.z - box.x, 0.f));
        }
        
        box.y = start_point + offset + element->margin.y;
        box.w = box.y + size.y;
        
        offset += size.y + element->margin.y + element->margin.w;
        offset += spaceBetweenElements;

        boxes.push_back(box);
    }

    return boxes;
}

template class ListLayout<ListDirection::RowCentered>;
template class ListLayout<ListDirection::ColumnCentered>;
