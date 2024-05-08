# Coding Guidelines

## Formatting

The easiest way to get the correct formatting is setting up `clang-format-14`. With the provided `.clang-format` file the files then can be formatted correctly.
On unix-based system there are helper scripts in the `scripts` folder:
- `check-format-cpp.sh`: checks if all C++ files are correctly formatted
- `format-cpp.sh`: formats all C++ files according to the formatting guidelines

## Naming conventions

The following naming conventions should be followed when contributing. These conventions can be checked with the `clang-tidy` using the provided `.clang-tidy` file. Please be aware that our exception for Qt slots unfortunately does not allow automatic testing. But most IDEs support `clang-tidy` which should lead to (visual) warnings.

- filenames: `classname.h`/`classname.cpp`
- class names: `CamelCase`
- function names: `camelBack`
- parameter names: `camelBack`
- macro defintions: `UPPER_CASE`
- member variable prefix: `m`
- member variable names: `CamelCase` -> results in `mCamelCase`
- struct names: `CamelCase`
- variable names: `camelBack`
- global constants names: `UPPER_CASE`
- namespace names: `camelBack`

Exceptions:
- slot names: `on_snake_case`

## Commenting Code

For documenting our code we use [`Doxygen`](https://www.doxygen.nl/manual/docblocks.html). It can later by exported in various formats. We decided to use the following conventions, these should be added in the `.cpp` files:

```cpp
/**
@brief Here is a short description in one line.

Here is the place for a longer description. This can also span over multiple sentences.

@see otherClass
@todo a todo
@bug a known bug
@warning warning in usage (c will be overwritten!)


@param[in] a Description of input parameter a
@param[out] b Description of output parameter b
@param[in,out] c Description of in-/output parameter c
@return Description of the return value
 */
int Petrack::myFunction(int a, int b, myClass& c){...}

/** 
 *  Long description of the member
 */
double foo;

int var; ///< Brief description after the member
```

It is also possible to add descriptive comments to classes, enums, files. The Doxygen comments then start with:
- `@class ClassName`
- `@enum EnumName`
- `@file fileName`

For adding formulas with LaTeX to the comments, you can use `f$ ... f$` for inline formulas and `f\[ ... f\]` for blocks.

## Refactoring Guidelines

In the course of finishing any task, you will most likely touch some legacy code. If you do more than renaming something, please also make sure to fix following issues:
- comments:
    - missing Doxygen description of function
    - translate comments to English
    - remove any commented out code, which is not needed anymore (thanks to git it is never lost completely)
- code style:
    - use range-based loops if possible
    - move variable declaration to the first usage and not top of scope (in particular loop indices!)
    - improve variable names
    - check if function can be made `const` or `static`


## Adding an element to the GUI

Since the GUI implementation might be slightly restructured in the foreseeable future, just a few hints.

* If it opens a new window, this window should have its own UI-File.
* Remember saving and loading everything that should be persistent (Like, if a specific checkbox is checked or not)
    * See: `PeTrack::openXml`
    * See: `PeTrack::saveXml`
    * XML here refers to the .pet-File which is a file in the XML format
* Layouts are important, make sure every Widget is part of one.
* Menu-Entries are in the code, not in the UI-Files
    * See `Petrack::createMenus`
    * See `Petrack::createActions`
