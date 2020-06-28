
#pragma once

#include "std.h"

#include "ui/UIElement.h"

class ContainerLayout
{
public:
    virtual vector<vec4> layoutElements(const UIElement* element, vec4 rect,
        const vector<shared_ptr<UIElement>>& elements,
        const hash_map<const UIElement*, UILayoutInfo>& layoutInfo) const = 0;
    
    virtual UILayoutInfo computeLayoutInfo(const UIElement* element, const vector<UILayoutInfo>& childLayout,
        const vector<shared_ptr<UIElement>>& elements) const = 0;
};

class OverlayLayout : public ContainerLayout
{
public:
    virtual vector<vec4> layoutElements(const UIElement* element, vec4 rect,
        const vector<shared_ptr<UIElement>>& elements,
        const hash_map<const UIElement*, UILayoutInfo>& layoutInfo) const override;
    
    virtual UILayoutInfo computeLayoutInfo(const UIElement* element, const vector<UILayoutInfo>& childLayout,
        const vector<shared_ptr<UIElement>>& elements) const override;
};

enum class ListDirection : uchar {
    Row, Column, RowReverse, ColumnReverse, RowCentered, ColumnCentered
};

// This is templated so most of the code will be optimized out (due to the constant nature of dir).
template<ListDirection dir>
class ListLayout : public ContainerLayout
{
public:
    float spaceBetweenElements;

    ListLayout(float _spaceBetweenElements)
        : spaceBetweenElements(_spaceBetweenElements) {}

    virtual vector<vec4> layoutElements(const UIElement* element, vec4 rect,
        const vector<shared_ptr<UIElement>>& elements,
        const hash_map<const UIElement*, UILayoutInfo>& layoutInfo) const override;
    
    virtual UILayoutInfo computeLayoutInfo(const UIElement* element, const vector<UILayoutInfo>& childLayout,
        const vector<shared_ptr<UIElement>>& elements) const override;
};
