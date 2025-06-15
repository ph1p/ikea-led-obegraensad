# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "matplotlib",
#     "numpy",
#     "requests",
# ]
# ///

import requests
import numpy as np
import matplotlib.pyplot as plt

def fetch_led_data(url):
    response = requests.get(url)
    response.raise_for_status()
    return response.content

def visualize_led_grid(data):
    # Expecting 256 bytes for a 16x16 grid
    if len(data) != 256:
        raise ValueError(f"Expected 256 bytes, got {len(data)}")
    grid = np.frombuffer(data, dtype=np.uint8).reshape((16, 16))
    plt.imshow(grid, cmap='gray', vmin=0, vmax=255)
    plt.title("16x16 LED Grid State")
    plt.axis('off')
    plt.show()

def main() -> None:
    url = "http://obegraensad.local/api/data"
    led_data = fetch_led_data(url)
    visualize_led_grid(led_data)


if __name__ == "__main__":
    main()
