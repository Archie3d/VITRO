namespace vitro {

ViewContainer::ViewContainer()
{
}

ViewContainer::~ViewContainer()
{
    notifyContextAboutToBeDeleted();

    // The view must be deleted before the context, otherwise JS context will leak.
    view.reset();
    context.reset();
}

void ViewContainer::addListener(Listener* listener)
{
    jassert(listener != nullptr);
    listeners.add(listener);
}

void ViewContainer::removeListener(Listener* listener)
{
    jassert(listener != nullptr);
    listeners.remove(listener);
}

void ViewContainer::setLocalDirectory(const File& dir)
{
    localDir = dir;

    if (context) {
        context->getLoader().setLocalDirectory(localDir);
    }
}

void ViewContainer::loadFromResource(const String& xmlLocation,
                                     const String& cssLocation,
                                     const String& scriptLocation)
{
    notifyContextAboutToBeDeleted();

    // Delete the view before the context
    view.reset();

    context = std::make_unique<vitro::Context>();

    listeners.call(&Listener::onContextCreated, context.get());

    auto& loader{context->getLoader() };
    loader.setLocalDirectory(localDir);

    if (cssLocation.isNotEmpty()) {
        auto styleImporter = [&](const String& location) -> String {
            return loader.loadText(location);
        };

        auto& stylesheet{ context->getStylesheet() };

        stylesheet.clear();
        stylesheet.populateFromString(loader.loadText(cssLocation), styleImporter);
    }

    // Create a new view before evaluating the script
    view = std::dynamic_pointer_cast<vitro::View>(context->getElementsFactory().createElement(vitro::View::tag));
    addAndMakeVisible(view.get());

    if (scriptLocation.isNotEmpty()) {
        const String script{ loader.loadText(scriptLocation) };

        if (script.isNotEmpty()) {
            context->eval(script, scriptLocation);
#if JUCE_DEBUG
            context->dumpError();
#endif
        }
    }

    // This will remove all current children in the view
    view->populateFromXmlResource(xmlLocation);

    listeners.call([&](Listener& listener){ listener.onViewLoaded(view.get()); });

    // Need to call resized so that the new view gets adjusted to the container
    resized();
}

void ViewContainer::resized()
{
    if (view)
        view->setBounds(getLocalBounds());
}

void ViewContainer::notifyContextAboutToBeDeleted()
{
    if (context == nullptr)
        return;

    listeners.call(&Listener::onContextAboutToBeDeleted, context.get());
}


} // namespace vitro
