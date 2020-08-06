
#include "ui/TextField.h"
#include "ui/UISystem.h"

inline bool isShiftDown(shared_ptr<UISystem> ui) {
    return ui->isKeyDown(340) || ui->isKeyDown(344);
}

inline bool isControlDown(shared_ptr<UISystem> ui) {
    return ui->isKeyDown(341) || ui->isKeyDown(345);
}

inline vec2 getPointFromPrepointId(const Font::StringLayout& layout, uint id, const vec4& layoutBox,
    Font::Alignment alignment)
{
    if(id < layout.text.size()) {
        return layout.prePoints[id];
    } else if(layout.advancePoints.size() > 0) {
        return layout.advancePoints.back();
    } else {
        float xVal = 0;
        switch(alignment) {
        case Font::Alignment::Right:
            xVal = layoutBox.z - layoutBox.x;
            break;
        case Font::Alignment::Center:
            xVal = (layoutBox.z - layoutBox.x) * 0.5f;
            break;
        default:
            xVal = 0;
            break;
        }
        return vec2(xVal, layout.maxAscent);
    }
}

inline vec2 getPointFromAdvanceId(const Font::StringLayout& layout, uint id)
{
    return layout.advancePoints[id];
}

inline uvec2 orderRange(const uvec2& range)
{
    if(range.x <= range.y) {
        return range;
    } else {
        return uvec2(range.y, range.x);
    }
}

inline string replaceRange(string str, uvec2 range, uchar c)
{
    if(c) {
        return str.substr(0, range.x) + (char)c + str.substr(range.y);
    } else {
        return str.substr(0, range.x) + str.substr(range.y);
    }
}

TextField::TextField()
{
    blocksInteractive = true;
    canBeFocused = true;
}

void TextField::sync()
{
    if(!box) {
        box = make_shared<Box>();
        box->layoutAlgorithm = new OverlayLayout();
        box->blocksInteractive = false;
        addElementChild(box);
    }
    UIElement::LayoutDetails ld = box->getLayoutDetails();
    ld.padding = interiorPadding;
    box->setLayoutDetails(ld);
    box->cornerRadii = cornerRadii;

    if(!textBox) {
        textBox = make_shared<Container>();
        textBox->layoutAlgorithm = new OverlayLayout();
        textBox->blocksInteractive = false;
        addElementChild(textBox);
    }
    ld = textBox->getLayoutDetails();
    ld.padding = interiorPadding;
    textBox->setLayoutDetails(ld);

    if(!text) {
        text = make_shared<Text>();
        text->setFontSize(fontSize);
        text->setLineSpacing(1.0f);
        ld = text->getLayoutDetails();
        ld.anchors = vec4(0,0,0,0);
        text->setLayoutDetails(ld);
        text->blocksInteractive = false;
        textBox->addChild(text);
    }
    text->font = font;
    text->setUnboundedLayout(useUnboundedLayout);
    text->setTextAlignment(alignment);
    text->setText(value);

    if(!textPulse) {
        textPulse = make_shared<Box>();
        ld = textPulse->getLayoutDetails();
        ld.anchors = vec4(0,0,0,0);
        textPulse->setLayoutDetails(ld);
        textPulse->blocksInteractive = false;
        textBox->addChild(textPulse);
    }
    ld = textPulse->getLayoutDetails();
    ld.size.x = pulseWidth;
    textPulse->setLayoutDetails(ld);
    
    if(!dragBox) {
        dragBox = make_shared<Box>();
        dragBox->blocksInteractive = false;
        // We will manage rendering the drag box ourselves.
    }
    dragBox->colour = dragColour;
}

void TextField::setValue(string _value)
{
    value = _value;
    if(text) {
        text->setText(_value);
    }
}

void TextField::setTextPoint(uint point, bool selectRange)
{
    if(point > value.size()) {
        point = (uint)value.size();
    }
    if(!selectRange) {
        textPoints.x = point;
    }
    textPoints.y = point;
    if(!text || !textBox) {
        textOffset = vec2(0,0);
        return;
    }
    const Font::StringLayout& layout = text->getTextLayout();
    if(layout.text.size() == 0) {
        textOffset = vec2(0,0);
        return;
    }
    vec4 charBounds;
    if(point < layout.text.size()) {
        // Use the pre and advance points to define the character bounds.
        charBounds = vec4(layout.prePoints[point], layout.advancePoints[point]);
        // Check if this character ends the line, in which case treat it like a space.
        if(charBounds.y != charBounds.w) {
            charBounds.z = charBounds.x + layout.spaceWidth;
            charBounds.w = charBounds.y;
        }
    } else {
        // use the last character's advance point.
        charBounds = vec4(layout.advancePoints.back(), layout.advancePoints.back());
        // Give it the width of a space.
        charBounds.z += layout.spaceWidth;
    }
    // Update the bounds to the correct height for the line.
    charBounds.y -= layout.maxAscent;
    charBounds.w += layout.maxDescent;
    // Move the character bounds to be in global space instead of relative to the text box.
    charBounds += vec4(text->getLayoutBox().x, text->getLayoutBox().y,
        text->getLayoutBox().x, text->getLayoutBox().y);

    // Compute the distance of each character edge to the box edges (ignoring padding).
    charBounds -= textBox->getPaddedLayoutBox();
    // We flip the top and left edges since we want positive distance to mean distance from outer bound.
    charBounds *= vec4(-1,-1,1,1);
    // Now we only care about moving the text box if the character is out of bounds, so clamp the distances to be +
    charBounds = vec4(glm::max(0.f, charBounds.x), glm::max(0.f, charBounds.y),
        glm::max(0.f, charBounds.z), glm::max(0.f, charBounds.w));

    textOffset.x += charBounds.x - charBounds.z;
    textOffset.y += charBounds.y - charBounds.w;
}

void TextField::movePointerLeft(bool select)
{
    if(!select && textPoints.x != textPoints.y) {
        setTextPoint(glm::min(textPoints.x, textPoints.y));
    } else {
        setTextPoint(glm::max((uint)1, textPoints.y) - 1, select);
    }
    currentState = Typing;
}

void TextField::movePointerRight(bool select)
{
    if(!select && textPoints.x != textPoints.y) {
        setTextPoint(glm::max(textPoints.x, textPoints.y));
    } else {
        setTextPoint(glm::min((uint)value.size(), textPoints.y + 1), select);
    }
    currentState = Typing;
}

enum TFCharType { Word, NewLine, Whitespace, Special };
TFCharType getTFCharType(uchar c)
{
    if('0' <= c && c <= '9' || 'A' <= c && c <= 'Z' || 'a' <= c && c <= 'z' || c == '_') {
        return Word;
    } else if(c == '\n') {
        return NewLine;
    } else if(c == ' ' || c == '\t') {
        return Whitespace;
    } else {
        return Special;
    }
}
                    
void TextField::movePointerLeftToken(bool select)
{
    uint curr = getTextPoint();
    if(curr == 0) {
        return;
    }
    TFCharType type;
    curr -= 1;
    while(curr && (type = getTFCharType(value[curr])) == Whitespace) {
        curr -= 1;
    }
    if(curr == 0 || type == NewLine) {
        setTextPoint(curr, select);
        return;
    }
    curr -= 1;
    TFCharType cmpType;
    while(curr && (cmpType = getTFCharType(value[curr])) == type) {
        curr -= 1;
    }
    curr += (cmpType != type);
    setTextPoint(curr, select);
}

void TextField::movePointerRightToken(bool select)
{
    uint curr = getTextPoint();
    if(curr == value.size()) {
        return;
    }
    TFCharType type;
    while(curr < value.size() && (type = getTFCharType(value[curr])) == Whitespace) {
        curr += 1;
    }
    if(curr == value.size() || type == NewLine) {
        setTextPoint(curr + 1, select);
        return;
    }
    curr += 1;
    TFCharType cmpType;
    while(curr < value.size() && (cmpType = getTFCharType(value[curr])) == type) {
        curr += 1;
    }
    setTextPoint(curr, select);
}

void TextField::movePointerDown(bool select)
{
    const Font::StringLayout& layout = text->getTextLayout();
    vec2 point = getPointFromPrepointId(layout, getTextPoint(), getLayoutBox(), alignment);
    point.y += layout.lineHeight;
    uint newTextPoint = layout.getCharacterAtPoint(point);
    setTextPoint(newTextPoint, select);
}

void TextField::movePointerUp(bool select)
{
    const Font::StringLayout& layout = text->getTextLayout();
    vec2 point = getPointFromPrepointId(layout, getTextPoint(), getLayoutBox(), alignment);
    point.y -= layout.lineHeight;
    uint newTextPoint = layout.getCharacterAtPoint(point);
    setTextPoint(newTextPoint, select);
}

void TextField::movePointerPageUp(bool select)
{
    const Font::StringLayout& layout = text->getTextLayout();
    vec2 point = getPointFromPrepointId(layout, getTextPoint(), getLayoutBox(), alignment);
    point.y -= text->getLayoutBox().w - text->getLayoutBox().y;
    uint newTextPoint = layout.getCharacterAtPoint(point);
    setTextPoint(newTextPoint, select);
}

void TextField::movePointerPageDown(bool select)
{
    const Font::StringLayout& layout = text->getTextLayout();
    vec2 point = getPointFromPrepointId(layout, getTextPoint(), getLayoutBox(), alignment);
    point.y += text->getLayoutBox().w - text->getLayoutBox().y;
    uint newTextPoint = layout.getCharacterAtPoint(point);
    setTextPoint(newTextPoint, select);
}

uint getTextPointLine(const Font::StringLayout& layout, uint textPoint)
{
    uint i;
    for(i = 0; i < layout.lineData.size(); i++) {
        const uvec4& line = layout.lineData[i];
        // If the text point is out of range of the line, move to the next line.
        if(line.y >= textPoint + (i == 0) || textPoint > line.w) {
            continue;
        }
        // Otherwise, we've found the correct line.
        return i;
    }
    return i;
}

void TextField::movePointerHome(bool select)
{
    uint targetPoint = getTextPoint();
    const Font::StringLayout& layout = text->getTextLayout();
    uint line = getTextPointLine(layout, targetPoint);
    // If we don't find any valid lines something has gone very wrong, so just abort.
    if(line == layout.lineData.size()) {
        return;
    }

    uvec2 charRange(layout.lineData[line].y, layout.lineData[line].w);
    // Send us home!
    setTextPoint(charRange.x + (line != 0), select);
}

void TextField::movePointerEnd(bool select)
{
    uint targetPoint = getTextPoint();
    const Font::StringLayout& layout = text->getTextLayout();
    uint line = getTextPointLine(layout, targetPoint);
    // If we don't find any valid lines something has gone very wrong, so just abort.
    if(line == layout.lineData.size()) {
        return;
    }

    uvec2 charRange(layout.lineData[line].y, layout.lineData[line].w);
    setTextPoint(charRange.y, select);
}

void TextField::backspaceSelected()
{
    uvec2 ordered = orderRange(textPoints);
    if(ordered.x == ordered.y) {
        // Backspacing at character 0 doesn't do anything.
        if(ordered.x == 0) {
            return;
        }
        ordered.x -= 1;
    }
    setTextPoint(ordered.x);
    setValue(replaceRange(value, ordered, 0));
    currentState = Typing;
}

void TextField::deleteSelected()
{
    uvec2 ordered = orderRange(textPoints);
    if(ordered.x == ordered.y) {
        // Deleting at last character doesn't do anything.
        if(ordered.y == value.size()) {
            return;
        }
        ordered.y += 1;
    }
    setTextPoint(ordered.x);
    setValue(replaceRange(value, ordered, 0));
    currentState = Typing;
}

UIColour::Type convert(TextField::State state)
{
    switch(state)
    {
    case TextField::Default:
        return UIColour::Base;
    case TextField::Hovered:
        return UIColour::Hovered;
    default: // case TextField::Typing or case TextField::Dragging
        return UIColour::Active;
    }
}

void TextField::renderDragBoxes(vec4 mask, vec2 surfaceSize)
{
    // No drag boxes to render if we aren't dragging or if the drag points haven't moved off each other yet.
    if(textPoints.x == textPoints.y) {
        return;
    }

    // Ordered points will have x be less than or equal to y. The range x -> y is inclusive.
    uvec2 orderedPoints = orderRange(textPoints) - uvec2(0, 1);
    const Font::StringLayout& layout = text->getTextLayout();
    for(const uvec4& line : layout.lineData) {
        // We will use prePoints for lineStart, and advancePoints for lineEnd;
        // The line starts on the first character of the line, skipping the new line if present.
        uint lineStart = line.y + (line.y != line.w && layout.text[line.y] == '\n');
        uint lineEnd = line.w - 1;

        vec2 startPoint;
        vec2 endPoint;

        // This can only occur on the first line if the first line is empty.
        if(line.y == line.w) {
            // The 0 index must be selected, otherwise the line is excluded.
            if(orderedPoints.x <= 0 && 0 <= orderedPoints.y) {
                startPoint = layout.prePoints[0] - vec2(0, layout.maxAscent);
                endPoint = layout.prePoints[0] + vec2(layout.spaceWidth, layout.maxDescent);
            } else {
                continue;
            }
        } else {
            uint clipStart = glm::max(lineStart, orderedPoints.x);
            uint clipEnd = glm::min(lineEnd, orderedPoints.y);
            // If the clip area is inconsistent, this line is not part of our selection.
            if(clipStart > clipEnd + 1) {
                continue;
            }
            
            startPoint = getPointFromPrepointId(layout, clipStart, getLayoutBox(), alignment);
            endPoint = getPointFromAdvanceId(layout, clipEnd);
            if(clipEnd < orderedPoints.y && clipEnd + 1 < layout.text.size()
                && layout.text[clipEnd + 1] == '\n') {
                endPoint.x += layout.spaceWidth;
            }

            startPoint.y -= layout.maxAscent;
            endPoint.y += layout.maxDescent;
        }

        vec2 boxTL = vec2(text->getLayoutBox().x, text->getLayoutBox().y);
        startPoint += boxTL;
        endPoint += boxTL;
        dragBox->updateLayouts(vec4(startPoint, endPoint), false);
        dragBox->render(intersect_boxes(mask, getLayoutBox()), surfaceSize);
    }
}

void TextField::render(vec4 mask, vec2 surfaceSize)
{
    if(maskChildren) {
        mask = intersect_boxes(mask, useUnpaddedBoxAsMask ? getLayoutBox() : getPaddedLayoutBox());
    }

    box->render(mask, surfaceSize);
    renderDragBoxes(mask, surfaceSize);
    textBox->render(mask, surfaceSize);
}

void TextField::update(float delta, vec4 mask, shared_ptr<UISystem> ui)
{
    WrapperElement::update(delta, mask, ui);

    font = text->font;
    
    shared_ptr<UIElement> focus = ui->getFocusedElement();

    switch(currentState)
    {
    case Default:
        if(focus.get() == this) {
            currentState = Hovered;
        }
        break;
    case Hovered:
        if(focus.get() != this) {
            currentState = Default;
        } else if(ui->isMousePressed(UISystem::MouseButton::LMB)) {
            currentState = Dragging;
            ui->focusLocked = true;
            setTextPoint(getTextPointFromPosition(ui->getMousePoint()));
            ui->addTypingListener(this);
        } else if(ui->isAcceptPressed()) {
            currentState = Typing;
            ui->focusLocked = true;
            setTextPoint((uint)value.size());
            ui->addTypingListener(this);
        }
    }
    
    if(currentState == Typing || currentState == Dragging) {
        vec4 validClickRegion = intersect_boxes(mask, getLayoutBox());
        // If we just clicked.
        if(ui->isMousePressed(UISystem::MouseButton::LMB)) {
            // If the click is inside out box, start a new drag.
            if(isPointInBox(validClickRegion, ui->getMousePoint())) {
                // Figure out where to put the pointer.
                uint point = getTextPointFromPosition(ui->getMousePoint());
                setTextPoint(point, isShiftDown(ui));
                currentState = Dragging;
                pulsing = true;
                pulseTime = 0;
            } else {
                // Otherwise we must be trying to focus something else.
                ui->focusLocked = false;
                ui->removeTypingListener(this);
                // Force an update so whatever we are clicking on receives the click.
                ui->forceUpdate();
                currentState = Default;
            }
        } else if(currentState == Dragging && ui->isMouseDown(UISystem::MouseButton::LMB)) {
            // Otherwise, if we're continuing a drag, update the drag endpoint.
            uint point = getTextPointFromPosition(ui->getMousePoint());
            setTextPoint(point, true);
        }

        if(currentState == Dragging && ui->isMouseReleased(UISystem::MouseButton::LMB)) {
            currentState = Typing;
        }
    }

    bool needsLayoutUpdate = false;
    bool needsDetailsUpdate = false;

    UIElement::LayoutDetails ld = text->getLayoutDetails();

    if(ld.position != textOffset) {
        ld.position = textOffset;
        needsDetailsUpdate = true;
        needsLayoutUpdate = true;
    }

    vec2 size = getBoxSize(getPaddedLayoutBox());
    if(ld.size != size) {
        ld.size = size;
        needsDetailsUpdate = true;
        needsLayoutUpdate = true;
    }

    if(needsDetailsUpdate) {
        needsDetailsUpdate = false;
        text->setLayoutDetails(ld, false);
    }

    shared_ptr<Font> fontPtr = font.resolve(Deferred);
    float newVal;
    if(fontPtr) {
        newVal = ((float)fontPtr->maxAscent + (float)fontPtr->maxDescent) * fontPtr->getPixelUnit(fontSize);
    } else {
        newVal = 0;
    }
    ld = textPulse->getLayoutDetails();
    if(ld.size.y != newVal) {
        ld.size.y = newVal;
        needsDetailsUpdate = true;
        needsLayoutUpdate = true;
    }

    const Font::StringLayout& layout = text->getTextLayout();
    vec2 pulsePos = getPointFromPrepointId(layout, getTextPoint(), getLayoutBox(), alignment);
    pulsePos.y -= layout.maxAscent;

    if(ld.position != pulsePos) {
        ld.position = pulsePos + textOffset;
        needsDetailsUpdate = true;
        needsLayoutUpdate = true;
    }
    if(needsDetailsUpdate) {
        textPulse->setLayoutDetails(ld, false);
    }

    if(needsLayoutUpdate) {
        textBox->updateLayouts(textBox->getLayoutBox(), false);
    }

    pulseTime = fmodf(pulseTime, pulseRate) + delta;
    if(pulseTime >= pulseRate) {
        pulseTime -= pulseRate;
        pulsing = !pulsing;
    }

    UIColour::Type type = convert(currentState);
    box->colour = backgroundColour.byType(type);
    text->colour = textColour.byType(type);
    textPulse->colour = textColour.byType(type) * ((float)(pulsing
        && (currentState == Typing || currentState == Dragging)));
}

uint TextField::getTextPointFromPosition(vec2 position) const
{
    if(!text || !textBox) {
        return 0;
    }
    const Font::StringLayout& layout = text->getTextLayout();
    vec4 layoutBox = textBox->getLayoutBox();
    const vec4& padding = textBox->getLayoutDetails().padding;
    position -= vec2(layoutBox.x + padding.x, layoutBox.y + padding.y);
    position -= text->getLayoutDetails().position;
    return layout.getCharacterAtPoint(position);
}

void TextField::onKeyTyped(uint key, shared_ptr<UISystem> ui)
{
    uchar c = (uchar) key;
    bool shift = isShiftDown(ui);
    if(c == 8) { // ASCII backspace
        backspaceSelected();
    } else if(c == 127) { // ASCII delete
        deleteSelected();
    } else if(c == 17) { // Left
        if(isControlDown(ui)) { movePointerLeftToken(shift); }
        else { movePointerLeft(shift); }
    } else if(c == 18) { // Right
        if(isControlDown(ui)) { movePointerRightToken(shift); }
        else { movePointerRight(shift); }
    } else if(c == 19) { // Down
        movePointerDown(shift);
    } else if(c == 20) { // Up
        movePointerUp(shift);
    } else if(c == 128) { // Page up
        movePointerPageUp(shift);
    } else if(c == 129) { // Page down
        movePointerPageDown(shift);
    } else if(c == 130) { // Home
        movePointerHome(shift);
    } else if(c == 131) { // End
        movePointerEnd(shift);
    } else { // This is a printable key.
        uvec2 ordered = textPoints.x <= textPoints.y ? textPoints : uvec2(textPoints.y, textPoints.x);
        value = replaceRange(value, ordered, c);
        setTextPoint(ordered.x + 1);
        text->setText(value);
        currentState = Typing;
    }
}
