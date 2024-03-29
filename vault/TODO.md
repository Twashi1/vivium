https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#memory-allocation
- [ ] Look into writing an allocator such that Vulkan object's memory is controlled by us
- [ ] May be possible to have one `close` method, since we could insert a special free function when using a custom allocator
- [ ] find new names for DescriptorSet and DescriptorLayout