namespace vitro {

ViewContainer::ViewContainer()
{
}

ViewContainer::~ViewContainer()
{
    // The view must be deleted before the context, otherwise JS context will leak.
    view.reset();
    context.reset();
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
    // Delete the view before the context
    view.reset();

    context = std::make_unique<vitro::Context>();

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

    // Need to call resized so that the new view gets adjusted to the container
    resized();
}

void ViewContainer::resized()
{
    if (view)
        view->setBounds(getLocalBounds());
}

} // namespace vitro
