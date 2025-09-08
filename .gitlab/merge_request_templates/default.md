**Add your decription here**

# Reviewer Checklist

## General code quality

- [ ] naming conventions are met (see .clang-tidy for detailed information)
- [ ] no static analyzer warnings in new code parts (e.g., use clang-tidy for checking)

## General usability

- [ ] old versions of pet-files are still loadable
- [ ] documentation updated (if applicable)

## Changelog

- [ ] `CHANGELOG.md` has been updated with key highlights

## Only if changes in UI

- [ ] new elements are also saved and loaded from pet-file
- [ ] check if tab order is still correct
- [ ] all new SpinBoxes are promoted
- [ ] new keybindings added to `Petrack::keyBindings()`
