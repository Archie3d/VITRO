namespace vitro {

struct Context::Impl
{
    Context& self;

    Loader loader{};
    Stylesheet stylesheet{};
    ElementsFactory elementsFactory;

    Impl(Context& ctx)
        : self{ ctx },
          elementsFactory(ctx)
    {
    }

    void initialize()
    {
        elementsFactory.registerDefaultElements();
    }

    void reset()
    {
        stylesheet.clear();
    }
};

//==============================================================================

Context::Context()
    : d{ std::make_unique<Impl>(*this) }
{
    d->initialize();
}

Context::~Context() = default;

const Loader& Context::getLoader() const
{
    return d->loader;
}

Loader& Context::getLoader()
{
    return d->loader;
}

const Stylesheet& Context::getStylesheet() const
{
    return d->stylesheet;
}

Stylesheet& Context::getStylesheet()
{
    return d->stylesheet;
}

const ElementsFactory& Context::getElementsFactory() const
{
    return d->elementsFactory;
}

ElementsFactory& Context::getElementsFactory()
{
    return d->elementsFactory;
}

void Context::initialize()
{
    d->initialize();
}

void Context::reset()
{
    d->reset();
}

} // namespace vitro
