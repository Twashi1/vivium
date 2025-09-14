# Taken from
# https://github.com/TheCherno/Hazel
import os
import sys
import subprocess
from pathlib import Path
from io import BytesIO
from urllib.request import urlopen

import downloadUtils

VULKAN_VERSION = "v1.4.321"
VULKAN_MINIMUM = "1.4"

def validateVulkanSDK():
    vulkanSDK = os.environ.get("VULKAN_SDK")

    if vulkanSDK is None:
        print(f"Vulkan SDK is not installed, attempting to install version {VULKAN_VERSION}")
        return installVulkanSDK()

    if VULKAN_MINIMUM not in vulkanSDK:
        print(f"Require vulkan version of at least {VULKAN_VERSION}");
        return installVulkanSDK()

    print(f"Located valid vulkan version at {VULKAN_SDK}")
    return True

def installVulkanSDK():
    permissionGranted = False
    reply = input(f"Install Vulkan SDK {VULKAN_VERSION} (REQUIRED)? [Y/n]").lower()

    if 'n' in reply:
        return False

    # TODO: switch between windows/linux install url
    vulkanInstallURL = f"https://sdk.lunarg.com/sdk/download/{VULKAN_VERSION}/windows/VulkanSDK-{VULKAN_VERSION}-Installer.exe";
    # Install path for vulkan
    # TODO: can this be seen by CMake?
    vulkanPath = f"/external/Vulkan/VulaknSDK-{VULKAN_VERSION}-Installer.exe"
    print(f"Downloading {vulkanInstallURL} to {vulkanPath}")
    downloadUtils.downloadFile(vulkanInstallURL, vulkanPath)
    os.startfile(os.path.abspath(vulkanPath))
    
    print("Re-run to validate")
    
    return True

if __name__ == "__main__":
    if validateVulkanSDK(): return

