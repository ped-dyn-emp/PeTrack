#!/usr/bin/env python3
"""Print the citation information for PeTrack, fetched from Zenodo.

Invoked by ``conf.py``, which captures stdout and embeds it in the docs.

The record is looked up by *concept id*, the identifier that stays the same
across all releases, rather than by searching for the project name. A name
search matches titles, descriptions and author names, so it can return an
unrelated record: the top text match for "PeTrack" is currently a paper by an
author of that surname, not this software.

Zenodo being briefly unreachable must not fail the documentation build, so this
always prints something and always exits successfully.
"""

import logging
import sys

from zenodo_bibtex_exporter import ZenodoBibtexError, get_bibtex

logger = logging.getLogger(__name__)

#: PeTrack on Zenodo. This is the concept id, which never changes between releases.
CONCEPT_ID = "5078176"

CONCEPT_DOI_URL = f"https://doi.org/10.5281/zenodo.{CONCEPT_ID}"


def get_latest_petrack_bibtex() -> str:
    """Return the BibTeX entry for the most recent release.

    Returns:
        A BibTeX entry, or a comment explaining why there is none.
    """
    try:
        entry = get_bibtex(CONCEPT_ID)
    except ZenodoBibtexError as error:
        logger.warning("No citation information available: %s", error)
        return (
            f"% Citation information could not be retrieved from Zenodo.\n"
            f"% It is available at {CONCEPT_DOI_URL}\n"
        )
    else:
        return entry


if __name__ == "__main__":
    logging.basicConfig(
        level=logging.INFO, format="%(levelname)s: %(message)s", stream=sys.stderr
    )
    # conf.py captures stdout, so diagnostics must not go there.
    print(get_latest_petrack_bibtex(), end="")
