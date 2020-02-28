# magicitems

A simple set of Qt / C++ classes that allow to create a debug UI for Qt apps.
Does nothing if -DNDEBUG is defined.

### Usage : in a paint() method

- Inherit from `MagicGraphicsItem`.
- `double x = magic("gain", 0.5, 0., 1.);` creates a slider with a value between 0. and 1.
- 0.5 will be the value used when the library is disabled.

### Usage : in a constructor

- Inherit from `MagicGraphicsItem`.
- `panPort->setPos(magic("panport", {3, 18}, {0, 0}, {15, 30}, [=] (auto p) { panPort->setPos(p); });`
- Same as before, `{3, 18}` will be the default position.