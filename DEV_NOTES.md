# Development Notes

This document is a quick reference guide for development conventions in this project. It covers code style, TODO formatting, and commonly used tags to ensure consistency and maintainability.

## Codestyle

The project adheres to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

## TODOs

TODOs should follow the format below for clarity and uniformity:

```cpp
// TODO(author): Brief message describing the task.
// Tags: tag1 tag2 tag3
// A longer explanation if necessary, with each line no longer than 80
// characters.
```

Tags and longer explanations are optional.

### Commonly Used Tags

These tags are used to categorize TODOs based on technical themes or concerns:

* `concurrency`: Marks tasks related to concurrent programming challenges.
* `lockfree`: Indicates tasks involving lock-free data structures or algorithms.
* `job`: Refers to tasks involving job scheduling or task management.