# Contributing to Documentation

We document PeTrack using Markdown files which are part of our git repository.
These are then processes by Sphinx into our documentation site. The website is
automatically deployed, once the changes on the source files are part of the
master branch. If you want to contribute to the documentation, some local setup
is needed.

## Environment Setup

First of all, you need to have cloned the repository (for how to do that, see the section [Get the code](../contribute/code_contribution/build_from_source.md#get-the-code) of the source code contribution documentation). Note that you **do not
need to build PeTrack**, you just need the repository for the documentation
files. Then you can edit the markdown files in the `docs/source/` directory. Any
texteditor can be used for this. If you do not have an preferred text editor, we
recommend VSCode (note that VSCode has a MyST-Markdown extension).

To be able to build the documentation locally to see how the resulting webpage
looks, you furthermore need to install Python. Once you have a working Python
installation, create a virtual environment with the command `python -m venv
/path/to/new/virtual/environment`. This environment needs to be activated in the
shell you'll use for building the documentation. You can activate the venv with
the command `<venv>\Scripts\activate.bat` on Windows or `source
<venv>/bin/activate` on Linux. Then we can install the needed dependencies for
building the documentation with the by changing the directory in the shell to
`docs` and running `pip install -r requirements.txt`. Now you can compile the
documentation by running `make.bat html` on Windows, or `make html` on Linux or
MacOS. For a smoother local developement experience, you can install
`sphinx-build` via the command `pip install sphinx-build`. `sphinx-build` can be
executed as `sphinx-build ./source ./build/_html`. This will open a webpage
which is accessible at `http://127.0.0.1:8000` and which is automatically
updated when you change a file.

We also build the documentation as part of our CI. So if the local build does
not work for some reason, you can still download the artifact of the
`build-docs` job of our CI from GitLab to get the html files and view them in
your browser.

## Markdown

We use [MyST Markdown](https://myst-parser.readthedocs.io/en/latest/index.html)
for writing our documentation. There are several online documents explaining the
syntax, see e.g. the [MyST Cheat
Sheet](https://jupyterbook.org/en/stable/reference/cheatsheet.html) of the
jupyterbook project. Of course, you can also look at the existing pages to check
how the syntax for some part of the documentation looks like.

