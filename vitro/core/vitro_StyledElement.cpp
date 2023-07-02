namespace vitro {

StyledElement::Style::Style()
    : PropertySet()
{
}

void StyledElement::Style::propertyChanged()
{
    // @todo Flag style and layout recalculation
}

//==============================================================================

StyledElement::StyledElement()
{
}

StyledElement::StyledElement(const Identifier& tag)
    : Element(tag)
{
}

} // namespace vitro
