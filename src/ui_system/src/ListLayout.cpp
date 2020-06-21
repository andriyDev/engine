
#include "ui/ContainerLayouts.h"

#define isHorizontal(dir) (dir == ListDirection::Row || dir == ListDirection::RowReverse\
    || dir == ListDirection::RowCentered)

#define isReversed(dir) (dir == ListDirection::ColumnReverse || dir == ListDirection::RowReverse)

inline vec2 marginSize(const vec4& margin)
{
    return vec2(margin.x + margin.z, margin.y + margin.w);
}

template<ListDirection dir>
vector<vec4> ListLayout<dir>::layoutElements(const UIElement* element, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements,
    const hash_map<const UIElement*, vec2>& desiredSizes) const
{
    float weightSum = 0.f;
    float basisProduct = 0.f;
    float mainSize = 0;
    for(shared_ptr<UIElement> element : elements) {
        vec2 size = desiredSizes.find(element.get())->second;
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

    vector<vec4> boxes;
    boxes.reserve(elements.size());
    float offset = 0;
    for(shared_ptr<UIElement> element : elements) {
        vec4 box;
        vec2 size = desiredSizes.find(element.get())->second;
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

        boxes.push_back(box);
    }

    return boxes;
}

template<ListDirection dir>
vec2 ListLayout<dir>::computeDesiredSize(const UIElement* element, const vector<vec2>& childSizes,
    const vector<shared_ptr<UIElement>>& elements) const
{
    // Our sum begins with the constant padding.
    vec2 sum = vec2(0,0);
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
        vec2 size = childSizes[i] + marginSize(elements[i]->margin);
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
    return sum;
}

template class ListLayout<ListDirection::Row>;
template class ListLayout<ListDirection::RowReverse>;
template class ListLayout<ListDirection::Column>;
template class ListLayout<ListDirection::ColumnReverse>;

template<>
vector<vec4> ListLayout<ListDirection::RowCentered>::layoutElements(const UIElement* element, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements,
    const hash_map<const UIElement*, vec2>& desiredSizes) const
{
    float mainSize = 0;
    for(shared_ptr<UIElement> element : elements) {
        vec2 size = desiredSizes.find(element.get())->second;
        mainSize += size.x + element->margin.x + element->margin.z;
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);
    float start_point = ((rect.x + rect.z) - mainSize) * 0.5f;

    vector<vec4> boxes;
    boxes.reserve(elements.size());
    float offset = 0;
    for(shared_ptr<UIElement> element : elements) {
        vec4 box;
        vec2 size = desiredSizes.find(element.get())->second;
        size.x += 1;
        
        box.x = start_point + offset + element->margin.x;
        box.z = box.x + size.x;
        
        offset += size.x + element->margin.x + element->margin.z;
        offset += spaceBetweenElements;

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

        boxes.push_back(box);
    }

    return boxes;
}

template<>
vector<vec4> ListLayout<ListDirection::ColumnCentered>::layoutElements(const UIElement* element, vec4 rect,
    const vector<shared_ptr<UIElement>>& elements,
    const hash_map<const UIElement*, vec2>& desiredSizes) const
{
    float mainSize = 0;
    for(shared_ptr<UIElement> element : elements) {
        vec2 size = desiredSizes.find(element.get())->second;
        mainSize += size.y + element->margin.y + element->margin.w;
    }
    mainSize += spaceBetweenElements * (elements.size() - 1);
    float start_point = ((rect.y + rect.w) - mainSize) * 0.5f;

    vector<vec4> boxes;
    boxes.reserve(elements.size());
    float offset = 0;
    for(shared_ptr<UIElement> element : elements) {
        vec4 box;
        vec2 size = desiredSizes.find(element.get())->second;
        size.y += 1;
        
        box.y = start_point + offset + element->margin.y;
        box.w = box.y + size.y;
        
        offset += size.y + element->margin.y + element->margin.w;
        offset += spaceBetweenElements;

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

        boxes.push_back(box);
    }

    return boxes;
}

template class ListLayout<ListDirection::RowCentered>;
template class ListLayout<ListDirection::ColumnCentered>;
