# Description

This repository contains some basic wrappers around the Vulkan library in order
to simplify its usage. These wrappers intentionally expose lot of features of
Vulkan and thus their usage is still verbose, but a lot of less than using the
raw C API.

In particular, I do not propose a high level API, but some useful tools to focus
on Vulkan semantics wthout the everbosity of filling structs even with default
params. 

Currently, these wrappers can be used to make compute programs, 3D usage will
come later.

## Inspirations

- https://vulkan-tutorial.com/Introduction
- https://github.com/Glavnokoman/vuh
