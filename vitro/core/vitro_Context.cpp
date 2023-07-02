namespace vitro {

struct Context::Impl
{
    Context& self;

    Loader loader{};
    Stylesheet stylesheet{};

    Impl(Context& ctx)
        : self{ ctx }
    {
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

void Context::reset()
{
    d->reset();
}

} // namespace vitro
