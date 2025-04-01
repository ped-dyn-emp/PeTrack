# Configuration file for the Sphinx documentation builder.

# -- Project information
import datetime
import subprocess

current_year = datetime.datetime.today().year

project = "PeTrack"
copyright = f"{current_year}, Forschungszentrum Jülich GmbH, IAS-7"

import os
import re

dir_path = os.path.dirname(os.path.realpath(__file__))

with open(f"{dir_path}/../../CMakeLists.txt") as file:
    for line in file:
        match = re.search(
            "project\(petrack LANGUAGES CXX VERSION ([0-9]+\.[0-9]+\.[0-9]+)\)", line
        )
        if match is not None:
            version = match[1]
            break
    else:
        raise RuntimeError("Version not found")

# version = '0.1.0'
release = version

# -- General configuration

extensions = [
    "sphinx.ext.duration",
    "sphinx.ext.doctest",
    "sphinx.ext.intersphinx",
    "sphinx_design",
    "sphinx.ext.napoleon",
    "sphinx.ext.mathjax",
    "sphinx_favicon",
    "notfound.extension",
    "sphinx_copybutton",
    "myst_parser",
    "sphinx_tippy",
]

myst_enable_extensions = [
    "colon_fence",
    "html_image",
    "attrs_inline",
    "attrs_block",
    "dollarmath",
    "deflist",
    "substitution",
]

myst_url_schemes = {
    "http": None,
    "https": None,
    "wiki": "https://en.wikipedia.org/wiki/{{path}}#{{fragment}}",
    "doi": "https://doi.org/{{path}}",
}

myst_heading_anchors = 7


intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "sphinx": ("https://www.sphinx-doc.org/en/master/", None),
}
intersphinx_disabled_domains = ["std"]

templates_path = ["_templates"]

# -- Options for HTML output

html_theme = "sphinx_book_theme"
html_static_path = ["_static"]

html_logo = "_static/logo.png"
html_favicon = "_static/favicon.svg"

html_css_files = ["css/custom.css", "css/tippy.css"]
html_js_files = ["js/custom.js"]
html_context = {"default_mode": "light"}

html_theme_options = {
    "show_nav_level": 5,
    "use_fullscreen_button": False,
    "use_issues_button": False,
    "use_download_button": False,
    "article_header_end": ["breadcrumbs", "toggle-secondary-sidebar"],
    "icon_links": [
        {
            "name": "GitLab",
            "url": "https://jugit.fz-juelich.de/ped-dyn-emp/petrack",
            "icon": "fa-brands fa-square-gitlab",
        },
        {
            "name": "DOI",
            "url": "https://zenodo.org/doi/10.5281/zenodo.5078176",
            "icon": "https://zenodo.org/badge/DOI/10.5281/zenodo.5078176.svg",
            "type": "url",
        },
    ],
    "show_toc_level": 3,
}

html_sidebars = {
    "**": ["navbar-logo", "icon-links", "search-field", "sbt-sidebar-nav.html"]
}

# -- Options for EPUB output
epub_show_urls = "footnote"

# Inject PeTrack Version and Software Citation into docs
def generate_bibtex():
    result = subprocess.run(["python", "_scripts/generate-bibtex.py"], capture_output=True, text=True)
    return result.stdout.strip()

bibtex = generate_bibtex()

# Add the code block here, otherwise the substitution will not work
citation_block = f"""\
```
{bibtex}
```
"""

myst_substitutions = {
    'petrack_version': release,
    'petrack_citation': citation_block, 
}
