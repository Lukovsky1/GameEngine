#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
/* Provides functions, structures and enumerations */
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>

#include <cstring>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <fstream>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

using namespace std;

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

const vector<char const*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class HelloTriangleApplication {
public:
    void run();

private:
    vk::raii::Context  context;
    vk::raii::Instance instance = nullptr;
    GLFWwindow* window = nullptr;

    std::vector<const char*> requiredDeviceExtension = {};
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;
    vk::raii::Queue graphicsQueue = nullptr;
    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::raii::SurfaceKHR surface = nullptr;
    vk::raii::Queue queue = nullptr;
    vk::raii::Queue transferQueue = nullptr;

    uint32_t graphicsQueueIndex = ~0;
    uint32_t transferQueueIndex = ~0;

    vk::raii::SwapchainKHR swapChain = nullptr;
    std::vector<vk::Image> swapChainImages;
    vk::SurfaceFormatKHR   swapChainSurfaceFormat;
    vk::Extent2D           swapChainExtent;

    std::vector<vk::raii::ImageView> swapChainImageViews;

    void createDescriptorSetLayout();
    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
    vk::raii::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::raii::DescriptorSet> descriptorSets;

    void createDescriptorSets();

    vk::raii::PipelineLayout pipelineLayout = nullptr;
    vk::raii::Pipeline graphicsPipeline     = nullptr;

    vk::raii::CommandPool commandPool = nullptr;
    std::vector<vk::raii::CommandBuffer> commandBuffers;
    vk::raii::CommandPool transferCommandPool = nullptr;

    vk::raii::DeviceMemory vertexBufferMemory = nullptr;
    vk::raii::Buffer vertexBuffer = nullptr;
    vk::raii::Buffer       indexBuffer        = nullptr;
    vk::raii::DeviceMemory indexBufferMemory  = nullptr;

    std::vector<vk::raii::Buffer>       uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
    std::vector<void *>                 uniformBuffersMapped;

    std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
    std::vector<vk::raii::Fence>     inFlightFences;
    uint32_t frameIndex = 0;
    bool framebufferResized = false;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    void createInstance();
    std::vector<const char*> getRequiredInstanceExtensions();
    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);
    void createLogicalDevice();
    void createSurface();
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes);
    vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities);
    uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities);
    void createSwapChain();
    void createImageViews();
    void createGraphicsPipeline();
    std::vector<char> readFile(const std::string& filename);
    vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
    void createCommandPool();
    void createCommandBuffers();
    void recordCommandBuffer(uint32_t imageIndex);
    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const;
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void updateUniformBUffer(uint32_t currentImage);
    void copyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size);
    void copyBuffer(vk::raii::Buffer & srcBuffer, vk::raii::Buffer & gistBuffer, vk::DeviceSize size);
    void transitionImageLayout(uint32_t imageIndex,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask);
    void drawFrame();
    void createSyncObjects();

    void recreateSwapChain();
    void cleanupSwapChain();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;


};
