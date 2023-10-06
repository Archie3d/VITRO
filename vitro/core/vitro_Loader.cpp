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

String Loader::loadText(const String& location) const
{
    if (hasLocalPath) {
        auto file{ localPath.getChildFile(location) };

        if (file.existsAsFile())
            return file.loadFileAsString();
    }

#if JUCE_TARGET_HAS_BINARY_DATA
    if (auto* resourceName{ getBinaryDataResourceNameForFile(location) }) {
        int size{};

        if (const auto* data{ BinaryData::getNamedResource(resourceName, size) })
            return String::fromUTF8(data, size);
    }
#endif

    return {};
}

std::unique_ptr<XmlElement> Loader::loadXML(const juce::String& location) const
{
    const auto str{ loadText(location) };
    return XmlDocument::parse(str);
}

juce::Image Loader::loadImage(const String& location) const
{
    const auto hash{ location.hashCode64() };
    auto image{ ImageCache::getFromHashCode(hash) };

    if (image.isValid())
        return image;

    if (hasLocalPath) {
        auto file{ localPath.getChildFile(location) };

        if (file.existsAsFile())
            image = ImageFileFormat::loadFrom(file);
    }

#if JUCE_TARGET_HAS_BINARY_DATA

    if (!image.isValid()) {
        if (auto* resourceName{ getBinaryDataResourceNameForFile(location) }) {
            int size{};

            if (const auto* data{ BinaryData::getNamedResource(resourceName, size) })
                image = ImageFileFormat::loadFrom(data, size_t(size));
        }
    }
#endif

    if (image.isValid())
        ImageCache::addImageToCache(image, hash);

    return image;
}

} // namespace vitro
