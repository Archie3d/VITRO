namespace vitro {
namespace css {

bool Selector::Attribute::isEmpty() const
{
    return op == Operator::None || name.isNull();
}

bool Selector::Attribute::match(const ValueTree& tree) const
{
    if (isEmpty())
        return true;

    const bool exists{ tree.hasProperty(name) };
    const auto val{ tree.getProperty(name).toString() };

    switch (op) {
        case Operator::None:     return true;
        case Operator::Defined:  return exists;
        case Operator::IsTrue:   return exists && bool(tree.getProperty(name));
        case Operator::Equals:   return exists && val == value;
        case Operator::Contains: return exists && val.contains(value);
        case Operator::Prefix:   return exists && (val == value || val.startsWith(value + "-"));
        case Operator::Begins:   return exists && val.startsWith(value);
        case Operator::Ends:     return exists && val.endsWith(value);
        default:
            jassertfalse;
    }

    return false;
}

String Selector::Attribute::toString() const
{
    switch (op)
    {
        case Operator::None:     return "";
        case Operator::Defined:  return "[" + name + "]";
        case Operator::IsTrue:   return ":" + name;
        case Operator::Equals:   return "[" + name + "=" + value + "]";
        case Operator::Contains: return "[" + name + "*=" + value + "]";
        case Operator::Prefix:   return "[" + name + "|=" + value + "]";
        case Operator::Begins:   return "[" + name + "^=" + value + "]";
        case Operator::Ends:     return "[" + name + "$=" + value + "]";
        default:
           jassertfalse;
    }

    return {};
}

//==============================================================================

} // namespace css
} // namespace vitro
