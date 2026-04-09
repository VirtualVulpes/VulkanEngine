#include "resource_manager.h"

#include <iostream>
#include <ktx.h>

#include "constants.h"
#include "tiny_gltf.h"
#include "vulkan_device.h"

ResourceManager::ResourceManager(const VulkanDevice& device) : device(device),
                                                               frameLayout(createFrameLayout()),
                                                               materialLayout(createMaterialLayout()),
                                                               objectLayout(createObjectLayout()),
                                                               descriptorPool(createDescriptorPool()),
                                                               textureSampler(createTextureSampler()) {
}

Mesh& ResourceManager::loadMesh(std::string_view name, const Assets::MeshData& data) {
    auto [it, inserted] = meshes.try_emplace(std::string{name}, device, data);
    return it->second;
}

Mesh& ResourceManager::loadMesh(std::string_view name) {
    std::filesystem::path path = Config::modelPath / name;

    Assets::MeshData data{};
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;


    if (!loader.LoadBinaryFromFile(&model, &err, &warn, path.string())) {
        if (!warn.empty()) std::cout << "glTF warning: " << warn << std::endl;
        if (!err.empty()) std::cout << "glTF error: " << err << std::endl;
        throw std::runtime_error("failed to load glTF model");
    }

    // returns a typed pointer to element i of an accessor, respecting byteStride
    auto accessorPtr = [&](const tinygltf::Accessor& accessor, size_t i) -> const unsigned char* {
        const auto& view = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[view.buffer];
        size_t stride = view.byteStride != 0
                            ? view.byteStride
                            : tinygltf::GetComponentSizeInBytes(accessor.componentType)
                              * tinygltf::GetNumComponentsInType(accessor.type);
        return buffer.data.data() + view.byteOffset + accessor.byteOffset + i * stride;
    };

    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.indices < 0) continue;

            const auto baseVertex = static_cast<uint32_t>(data.vertices.size());

            // vertices
            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];

            auto texCoordIt = primitive.attributes.find("TEXCOORD_0");
            const tinygltf::Accessor* uvAccessor =
                    texCoordIt != primitive.attributes.end()
                        ? &model.accessors[texCoordIt->second]
                        : nullptr;

            data.vertices.reserve(data.vertices.size() + posAccessor.count);

            for (size_t i = 0; i < posAccessor.count; ++i) {
                Vertex vertex{};

                const auto* pos = reinterpret_cast<const float *>(accessorPtr(posAccessor, i));
                vertex.pos = {pos[0], pos[1], pos[2]};

                if (uvAccessor) {
                    const auto* uv = reinterpret_cast<const float *>(accessorPtr(*uvAccessor, i));
                    vertex.uv = {uv[0], uv[1]};
                }

                vertex.color = {1.0f, 1.0f, 1.0f};
                data.vertices.push_back(vertex);
            }

            // indices
            const auto& indexAccessor = model.accessors[primitive.indices];
            data.indices.reserve(data.indices.size() + indexAccessor.count);

            for (size_t i = 0; i < indexAccessor.count; ++i) {
                const auto* src = accessorPtr(indexAccessor, i);
                uint32_t index = 0;

                switch (indexAccessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: index = *src;
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: index = *reinterpret_cast<const uint16_t *>(src);
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: index = *reinterpret_cast<const uint32_t *>(src);
                        break;
                    default: throw std::runtime_error("Unsupported index component type");
                }

                data.indices.push_back(baseVertex + index);
            }
        }
    }

    auto [it, inserted] = meshes.try_emplace(path.stem().string(), device, data);

    return it->second;
}

Texture& ResourceManager::loadTexture(std::string_view name) {
    std::filesystem::path path = Config::texturePath / name;

    ktxTexture2* ktxTex2;
    KTX_error_code result = ktxTexture2_CreateFromNamedFile(
        path.string().data(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex2);

    if (result != KTX_SUCCESS) {
        throw std::runtime_error("failed to load ktx texture image");
    }

    if (ktxTexture2_NeedsTranscoding(ktxTex2)) {
        ktxTexture2_TranscodeBasis(ktxTex2, KTX_TTF_BC7_RGBA, 0);
    }

    auto format = static_cast<vk::Format>(ktxTex2->vkFormat);
    if (format == vk::Format::eUndefined) {
        format = vk::Format::eR8G8B8A8Unorm;
    }

    // get texture dimensions and data
    uint32_t width = ktxTex2->baseWidth;
    uint32_t height = ktxTex2->baseHeight;

    ktxTexture* base = ktxTexture(ktxTex2);
    ktx_size_t size = ktxTexture_GetDataSize(base);
    ktx_uint8_t* raw = ktxTexture_GetData(base);

    auto mipLevels = ktxTex2->numLevels;

    Assets::TextureData data = {
        .pixels = std::vector(reinterpret_cast<std::byte *>(raw), reinterpret_cast<std::byte *>(raw) + size),
        .width = width,
        .height = height,
        .mipLevels = mipLevels,
        .format = format
    };

    data.mipOffsets.resize(mipLevels);
    for (uint32_t i = 0; i < mipLevels; i++) {
        ktx_size_t offset;
        ktxTexture2_GetImageOffset(ktxTex2, i, 0, 0, &offset);
        data.mipOffsets[i] = offset;
    }

    ktxTexture2_Destroy(ktxTex2);

    ImageConfig config{
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .memory = vk::MemoryPropertyFlagBits::eDeviceLocal,
        .aspect = vk::ImageAspectFlagBits::eColor,
    };

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = *descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &*materialLayout
    };
    auto [it, inserted] = textures.try_emplace(path.stem().string(), device, data, config, textureSampler,
                                               std::move(device.getDevice().allocateDescriptorSets(allocInfo).front()));

    return it->second;
}

vk::raii::DescriptorSetLayout ResourceManager::createFrameLayout() {
    vk::DescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex
    };
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = 1,
        .pBindings = &binding
    };
    return device.getDevice().createDescriptorSetLayout(layoutInfo);
}

vk::raii::DescriptorSetLayout ResourceManager::createMaterialLayout() {
    vk::DescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment
    };
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = 1,
        .pBindings = &binding
    };
    return device.getDevice().createDescriptorSetLayout(layoutInfo);
}

vk::raii::DescriptorSetLayout ResourceManager::createObjectLayout() {
    vk::DescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex
    };
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = 1,
        .pBindings = &binding
    };
    return device.getDevice().createDescriptorSetLayout(layoutInfo);
}

vk::raii::DescriptorPool ResourceManager::createDescriptorPool() {
    std::array poolSizes{
        vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = Config::MAX_FRAMES_IN_FLIGHT * (1 + Config::MAX_OBJECTS) // frame + object sets
        },
        vk::DescriptorPoolSize{
            .type = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = Config::MAX_TEXTURES
        }
    };

    vk::DescriptorPoolCreateInfo info{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = Config::MAX_FRAMES_IN_FLIGHT + Config::MAX_OBJECTS + Config::MAX_TEXTURES,
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    return device.getDevice().createDescriptorPool(info);
}

vk::raii::Sampler ResourceManager::createTextureSampler() {
    vk::PhysicalDeviceProperties properties = device.getPhysicalDevice().getProperties();
    vk::SamplerCreateInfo createInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::True,
        .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
        .compareEnable = vk::False,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = vk::LodClampNone,
    };
    return {device.getDevice(), createInfo};
}
