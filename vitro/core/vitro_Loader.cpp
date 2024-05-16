#if JUCE_TARGET_HAS_BINARY_DATA
#   include "BinaryData.h"
#endif

namespace vitro {

static const char* getBinaryDataResourceNameForFile(const String& filename)
{
#if JUCE_TARGET_HAS_BINARY_DATA
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        if (filename == BinaryData::originalFilenames[i])
            return BinaryData::namedResourceList[i];
    }
#endif

    return nullptr;
}

//==============================================================================

Loader::Loader()
{
}

void Loader::setLocalDirectory(const juce::File& dir)
{
    localPath = dir;
    hasLocalPath = localPath.isDirectory();
}

String Loader::loadText(const String& location)
{
    if (hasLocalPath) {
        auto file{ localPath.getChildFile(location) };

        if (file.existsAsFile()) {
            const auto& content{ file.loadFileAsString() };
            notifyOnResourceLoaded(location, file);
            return content;
        }
    }

#if JUCE_TARGET_HAS_BINARY_DATA
    if (auto* resourceName{ getBinaryDataResourceNameForFile(location) }) {
        int size{};

        if (const auto* data{ BinaryData::getNamedResource(resourceName, size) }) {
            const auto content{ String::fromUTF8(data, size) };
            notifyOnResourceLoaded(location);
            return content;
        }
    }
#endif

    return {};
}

std::unique_ptr<XmlElement> Loader::loadXML(const juce::String& location)
{
    const auto str{ loadText(location) };
    return XmlDocument::parse(str);
}

juce::Image Loader::loadImage(const String& location)
{
    const auto hash{ location.hashCode64() };
    auto image{ ImageCache::getFromHashCode(hash) };

    if (image.isValid())
        return image;

    if (hasLocalPath) {
        auto file{ localPath.getChildFile(location) };

        if (file.existsAsFile()) {
            image = ImageFileFormat::loadFrom(file);
            notifyOnResourceLoaded(location, file);
        }
    }

#if JUCE_TARGET_HAS_BINARY_DATA

    if (!image.isValid()) {
        if (auto* resourceName{ getBinaryDataResourceNameForFile(location) }) {
            int size{};

            if (const auto* data{ BinaryData::getNamedResource(resourceName, size) }) {
                image = ImageFileFormat::loadFrom(data, size_t(size));
                notifyOnResourceLoaded(location);
            }
        }
    }
#endif

    if (image.isValid())
        ImageCache::addImageToCache(image, hash);

    return image;
}

void Loader::addListener(Listener* listener)
{
    jassert(listener != nullptr);
    listeners.add(listener);
}

void Loader::removeListener(Listener* listener)
{
    jassert(listener != nullptr);
    listeners.remove(listener);
}

void Loader::notifyOnResourceLoaded(const String& location, const juce::File& file)
{
    listeners.call(&Listener::onResourceLoaded, location, file);
}

} // namespace vitro
