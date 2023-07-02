namespace vitro {

// This represents a location split into scheme and path.
struct SchemeAndPath
{
    String scheme;
    String path;
};

static SchemeAndPath splitLocation(const String& location)
{
    SchemeAndPath snp{};

    const auto separator{ location.indexOf("://") };

    if (separator < 0) {
        snp.path = location;
    } else {
        snp.scheme = location.substring (0, separator).trim();
        snp.path = location.substring (separator + 3).trim();
    }

    return snp;
}


static const char* getBinaryDataResourceNameForFile (const String& filename)
{
#if JUCE_TARGET_HAS_BINARY_DATA
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
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
    const auto snp {splitLocation (location)};

    if (hasLocalPath) {
        auto file{ localPath.getChildFile(snp.path) };

        if (file.existsAsFile())
            return file.loadFileAsString();
    }

#if JUCE_TARGET_HAS_BINARY_DATA
    if (auto* resourceName{ getBinaryDataResourceNameForFile(location) })
    {
        int size{};

        if (const auto* data{ BinaryData::getNamedResource(resourceName, size) })
            return String::fromUTF8(data, size);
    }
#endif

    return {};
}

std::unique_ptr<XmlElement> Loader::loadXML(const juce::String& location) const
{
    const auto str{ loadText (location) };
    return XmlDocument::parse (str);
}

juce::Image Loader::loadImage(const String& location) const
{
    const auto hash{ location.hashCode64() };
    auto image{ ImageCache::getFromHashCode (hash) };

    if (image.isValid())
        return image;

    const auto snp{ splitLocation (location) };

    if (hasLocalPath) {
        auto file{ localPath.getChildFile (snp.path) };

        if (file.existsAsFile())
            image = ImageFileFormat::loadFrom (file);
    }

#if JUCE_TARGET_HAS_BINARY_DATA

    if (!image.isValid()) {
        if (auto* resourceName{ getBinaryDataResourceNameForFile(location) }) {
            int size{};

            if (const auto* data{ BinaryData::getNamedResource(resourceName, size) })
                image = ImageFileFormat::loadFrom(data, size_t (size));
        }
    }
#endif

    if (image.isValid())
        ImageCache::addImageToCache(image, hash);

    return image;
}

} // namespace vitro
