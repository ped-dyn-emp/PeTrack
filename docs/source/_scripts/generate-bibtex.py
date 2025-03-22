#!/usr/bin/env python3

import time
import warnings

import requests

search_query = "PeTrack"
record_id = None


def fetch_data_with_retries(
    url, params=None, headers=None, max_retries=10, wait_time=2
):
    for attempt in range(max_retries):
        try:
            response = requests.get(url, params=params, headers=headers)
            response.raise_for_status()
            return response
        except requests.RequestException as e:
            warnings.warn(f"Attempt {attempt + 1} failed: {e}")
            time.sleep(wait_time)
    raise RuntimeError("All attempts to fetch data failed.")


def get_latest_petrack_bibtex():
    try:
        response = fetch_data_with_retries(
            "https://zenodo.org/api/records",
            params={"q": search_query, "all_versions": True, "sort": "mostrecent"},
        )
        records = response.json()["hits"]["hits"]

        if not records:
            raise RuntimeError("No records found for PeTrack.")

        latest_record_id = records[0]["id"]
        headers = {"accept": "application/x-bibtex"}
        response = fetch_data_with_retries(
            f"https://zenodo.org/api/records/{latest_record_id}", headers=headers
        )
        response.encoding = "utf-8"

        if response.status_code == 200:
            return response.text
        else:
            raise RuntimeError("Not found")
    except Exception as e:
        warnings.warn(f"An error occurred: {e}")


if __name__ == "__main__":
    petrack_bibtex = get_latest_petrack_bibtex()
    print(petrack_bibtex)
