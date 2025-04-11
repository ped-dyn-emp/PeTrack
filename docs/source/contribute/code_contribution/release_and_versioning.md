# Releases and Versioning

The version number should always be adapted when something in the project file (.pet) changes. Additional changes happen when a milestone is completed.  
Releases should happen when a specific version is used to extract the trajectories for a larger experiment, e.g., CroMa.

## Release To-Do List:
List of all things which need to be done in order to create a proper new release. The steps mentioned in
[Creating a release](https://jugit.fz-juelich.de/ped-dyn-emp/petrack/-/wikis/dev/Release-and-Versioning#creating-a-release) need also be done to create a citable version with DOI for any publications.

### Creating a release
- Adapt version in CMakeLists.txt
  - run `update_pet_files.py` to upate all .pet files in the repo
- Adapt zenodo.json if needed (new contributors)
- create tag for the new release and make some release notes in GitLab
- create release from new tag in GitHub! (needed for Zenodo DOI)

### After release:
- update any mention of the Zenodo DOI and/or BibTeX
    - ReadMe
    - about menu
    - PeTrack project page ( https://www.fz-juelich.de/en/ias/ias-7/services/software/petrack )
- upload correct installer into the Releases (GitHub, then link GitHub for GitLab, due to GitLab file size limits)
- create a version of the docs in ReadTheDocs


