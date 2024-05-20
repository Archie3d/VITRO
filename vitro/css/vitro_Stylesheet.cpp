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

bool Selector::Attribute::isOtherMoreImportant(const Attribute& other) const
{
    // @todo
    // This is a very dirty way of comparing attributes.
    // So far we just make sure that 'active' takes priority over 'hover'.

    if (name == attr::active)
        return false;

    if (name == attr::hover)
        return other.name == attr::active;

    if (op == Operator::None && other.op != Operator::None)
        return true;

    return false;
}

//==============================================================================

Selector::Selector(StringRef argTag, StringRef argClass, StringRef argId)
    : tag{ argTag },
      clazz{ argClass },
      id{ argId }
{
}

const bool Selector::operator ==(const Selector& other) const
{
    return tag == other.tag
        && clazz == other.clazz
        && id == other.id;
}

const bool Selector::operator !=(const Selector& other) const
{
    return tag != other.tag
        || clazz != other.clazz
        || id != other.id;
}

void Selector::addAttribute(const Selector::Attribute& attr)
{
    if (!attr.isEmpty())
        attributes.add(attr);
}

void Selector::addAttribute(Selector::Attribute&& attr)
{
    if (!attr.isEmpty())
        attributes.add(std::move(attr));
}

bool Selector::matchTag(const String& argTag) const
{
    return tag.isEmpty() || tag == argTag;
}

bool Selector::matchClasses(const StringArray& argClassArray) const
{
    if (clazz.isEmpty())
        return true;

    for (const auto& cl : argClassArray) {
        if (cl == clazz)
            return true;
    }

    return false;
}

bool Selector::matchId(const String& argId) const
{
    return id.isEmpty() || id == argId;
}

bool Selector::matchAttributes(const ValueTree& tree) const
{
    for (const auto& attr : attributes) {
        if (!attr.match(tree))
            return false;
    }

    return true;
}

bool Selector::match(const String& argTag, const StringArray& argClassArray, const String& argId) const
{
    return matchTag(argTag) && matchClasses(argClassArray) && matchId(argId);
}

bool Selector::match(const String& argTag, const StringArray& argClassArray, const String& argId, const ValueTree& tree) const
{
    return match(argTag, argClassArray, argId) && matchAttributes(tree);
}

bool Selector::match(const ValueTree& tree) const
{
    const static Identifier attr_class("class");
    const static Identifier attr_id("id");

    StringArray treeClasses{};

    const auto& cl{ tree.getProperty(attr_class) };

    if (cl.isString()) {
        treeClasses.addTokens(cl.toString(), " ", "");
    } else if (cl.isArray()) {
        for (int i = 0; i < cl.size(); ++i)
            treeClasses.add(cl[i].toString());
    }

    return match(tree.getType().toString(),
                 treeClasses,
                 tree.getProperty(attr_id).toString())
        && matchAttributes(tree);
}

bool Selector::isOtherMoreImportant(const Selector& other) const
{
    if (id.isEmpty() && other.id.isNotEmpty()) return true;
    if (other.id.isEmpty() && id.isNotEmpty()) return false;
    if (clazz.isEmpty() && other.clazz.isNotEmpty()) return true;
    if (other.clazz.isEmpty() && clazz.isNotEmpty()) return false;
    if (tag.isEmpty() && other.tag.isNotEmpty()) return true;
    if (other.tag.isEmpty() && tag.isNotEmpty()) return false;

    // We assume a selector with more attributes should take priority
    if (other.attributes.size() > attributes.size()) return true;

    if (other.attributes.size() == attributes.size()) {
        for (int i = 0; i < attributes.size(); ++i) {
            if (attributes.getReference(i).isOtherMoreImportant(other.attributes.getReference(i)))
                return true;
        }
    }

    return false;
}

String Selector::toString() const
{
    String str{ tag };

    if (clazz.isNotEmpty())
        str += "." + clazz;

    if (id.isNotEmpty())
        str += "#" + id;

    for (const auto& attr : attributes)
        str += attr.toString();

    return str;
}

//==============================================================================

Style::Style()
{
}

void Style::addSelector(const Selector& selector)
{
    selectors.add(selector);
}

void Style::addSelector(Selector&& selector)
{
    selectors.add(std::move(selector));
}

void Style::addExtendSelector(const Selector& selector)
{
    extendSelectors.add(selector);
}

void Style::addExtendSelector(Selector&& selector)
{
    extendSelectors.add(std::move(selector));
}

bool Style::hasProperty(const Identifier& name) const
{
    return properties.contains(name);
}

void Style::setProperty(const Identifier& name, const var& value)
{
    properties.set(name, value);
}

void Style::setProperty(const Identifier& name, var&& value)
{
    properties.set(name, std::move(value));
}

const var& Style::getProperty(const Identifier& name) const
{
    return properties[name];
}

const var& Style::operator[] (const Identifier& name) const
{
    return properties[name];
}

bool Style::match(const String& tag, const StringArray& classes, const String& id, const ValueTree& tree) const
{
    for (const auto& selector : selectors) {
       if (selector.match(tag, classes, id, tree))
           return true;
    }

    return false;
}

bool Style::match(const ValueTree& tree) const
{
    for (const auto& selector : selectors) {
        if (selector.match(tree))
            return true;
    }

    return false;
}

bool Style::match(const Selector& otherSelector) const
{
    for (const auto& selector : selectors) {
        if (selector == otherSelector)
            return true;
    }

    return false;
}

bool Style::isOtherMoreImportant(const Style& style) const
{
    if (const auto* thisSelector{ getMostImportantSelector() }) {
        if (const auto* otherSelector{ style.getMostImportantSelector() })
            return thisSelector->isOtherMoreImportant (*otherSelector);
    } else {
        if (const auto* otherSelector{ style.getMostImportantSelector() })
            return true;
    }

    return false;
}

const Selector* Style::getMostImportantSelector() const
{
    const Selector* selPtr{ nullptr };

    for (const auto& selector : selectors) {
        if (selPtr == nullptr || selPtr->isOtherMoreImportant(selector))
            selPtr = &selector;
    }

    return selPtr;
}

} // namespace css

//==============================================================================

// A stub void value that is returned when a property
// cannot be found in a stylesheet. This is required because
// the properties are returned by reference.
const static var voidVar{};

Stylesheet::Stylesheet()
    : macroDefinitions{}
    , styles{}
{
}

void Stylesheet::clear()
{
    styles.clear();
}

void Stylesheet::addStyle(const css::Style& style)
{
    styles.add (style);
}

void Stylesheet::addStyle(css::Style&& style)
{
    styles.add(std::move(style));
}

const var& Stylesheet::getProperty(const Identifier& name,
                                   const String& tag,
                                   const StringArray& classes,
                                   const String& id,
                                   const ValueTree& tree) const
{
    const css::Style* matchedStyle{ nullptr };
    const css::Style* matchedExtendStyle{ nullptr };

    for (const auto& style : styles) {
        if (style.match(tag, classes, id, tree)) {
            if (style.hasProperty(name)) {
                if (matchedStyle == nullptr || matchedStyle->isOtherMoreImportant(style))
                    matchedStyle = &style;
            } else {
                // Follow the extend selectors
                if (const css::Style* extendStyle{ matchExtendStyle(style, name) }) {
                    if (matchedStyle == nullptr || matchedStyle->isOtherMoreImportant(style)) {
                        matchedStyle = &style;
                        matchedExtendStyle = extendStyle;
                    }
                }
            }
        }
    }

    if (matchedExtendStyle != nullptr)
        return matchedExtendStyle->getProperty(name);

    if (matchedStyle != nullptr)
        return matchedStyle->getProperty(name);

    return voidVar;
}

const var& Stylesheet::getProperty(const Identifier& name, const ValueTree& tree) const
{
    const css::Style* matchedStyle{ nullptr };
    const css::Style* matchedExtendStyle{ nullptr };

    for (const auto& style : styles) {
        if (style.match(tree)) {
            if (style.hasProperty(name)) {
                if (matchedStyle == nullptr || matchedStyle->isOtherMoreImportant(style))
                    matchedStyle = &style;
            } else {
                // Follow the extend selectors
                if (const css::Style* extendStyle{ matchExtendStyle(style, name) }) {
                    if (matchedStyle == nullptr || matchedStyle->isOtherMoreImportant(style)) {
                        matchedStyle = &style;
                        matchedExtendStyle = extendStyle;
                    }
                }
            }
        }
    }

    if (matchedExtendStyle != nullptr)
        return matchedExtendStyle->getProperty(name);

    if (matchedStyle != nullptr)
        return matchedStyle->getProperty(name);

    return voidVar;
}

void Stylesheet::populateFromString(const String& text, const ImportFunction& importFunction)
{
    CSSParser parser(*this);
    parser.setImportFunction(importFunction);
    parser.fromString(text);
}

void Stylesheet::populateFromVar(const var& val, const ImportFunction& importFunction)
{
    if (val.isString()) {
        String str = val.toString();

        if (!str.startsWith("{"))
            str = "{" + str + "}";

        populateFromString(str, importFunction);

    } else if (val.isObject()) {
        if (auto* obj = val.getDynamicObject()) {
            css::Style style;
            style.addSelector({});

            const auto& properties = obj->getProperties();

            for (const auto& prop : properties)
                style.setProperty(prop.name, prop.value.toString());

            addStyle(std::move(style));
        }
    }
}

const css::Style* Stylesheet::matchExtendStyle(const css::Style& style, const Identifier& name) const
{
    for (const auto& extendSelector : style.getExtendSelectors()) {
        for (const auto& extendStyle : styles) {
            // Skip over the current style
            if (&extendStyle != &style) {
                if (extendStyle.match(extendSelector) && extendStyle.hasProperty(name))
                    return &extendStyle;
            }
        }
    }

    return nullptr;
}

} // namespace vitro
