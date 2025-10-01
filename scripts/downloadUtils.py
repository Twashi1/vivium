import sys
import os
import requests
import urllib

def downloadFile(url, filename):
    print(f"Downloading {url} to {filename}")

    os.makedirs(os.path.dirname(filename), exist_ok=True)

    # Create agent otherwise request is blocked
    req = urllib.request.Request(
        url,
        headers = {
            "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
            "AppleWebKit/537.36 (KHTML, like Gecko) "
            "Chrome/115.0 Safari/537.36"
            }
        )

    with urllib.request.urlopen(req) as response, open(filename, "wb") as f:
        fileSize = int(response.info().get("Content-Length", -1))
        downloaded = 0
        blockSize = 1024 * 64

        while True:
            buffer = response.read(blockSize)
            
            if not buffer:
                break

            f.write(buffer)

            downloaded += len(buffer)
            percent = downloaded * 100 / fileSize if fileSize > 0 else 0

            # Progress bar
            print(f"\r[{percent:.2f}%]", end="")

    print("\nDownload complete")

