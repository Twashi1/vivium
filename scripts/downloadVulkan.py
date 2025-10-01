# Taken from
# https://github.com/TheCherno/Hazel
# TODO: use RuntimeError or appropriate errors
import os
import sys
import subprocess
import platform
import tarfile
from pathlib import Path
from io import BytesIO
from urllib.request import urlopen

import downloadUtils

VULKAN_VERSION = "1.4.321.1"
VULKAN_MAJOR_MINIMUM = 1
VULKAN_MINOR_MINIMUM = 4

def validateVulkanSDK():
    try:
        result = subprocess.run(
                ["vulkaninfo"],
                capture_output=True,
                text=True,
                check=True
                )

        for line in result.stdout.splitlines():
            if "Vulkan Instance Version: " in line:
                versionString = line.split(": ")[1].strip()
                major, minor, *_ = versionString.split(".")

                if int(major) == VULKAN_MAJOR_MINIMUM and int(minor) >= VULKAN_MINOR_MINIMUM:
                    print("Vulkan version validated!")
                    return True
                else:
                    print("Vulkan version too old, expected at least {VULKAN_MAJOR_MINIMUM}.{VULKAN_MINOR_MINIMUM}, but got {major}.{minor}")
                    return False

        return False
    except Exception as e:
        print("Got error {e}, vulkan not installed correctly?")

        return installVulkanSDK()

    # TODO: validate that we can get glslc

def installWindows(installer):
    print("Running installer")

    try:
        subprocess.run([installer], check=True)
    except Exception as e:
        print(f"Couldn't run installer: {e}")

    print("Follow steps in the vulkan installer")

def installLinux(tarball, location):
    print(f"Extracting {tarball} to {location}")

    os.makedirs(location, exist_ok=True)

    with tarfile.open(tarball, "r:xz") as tar:
        tar.extractall(location)

    sdkPath = os.path.abspath(os.path.join(location, VULKAN_VERSION, "x86_64"))

    config = f"""
export VULKAN_SDK={sdkPath}
export PATH=$VULKAN_SDK/bin:$PATH
export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH
export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d
"""

    bashrcPath = os.path.join(os.path.expanduser("~"), ".bashrc")

    with open(bashrcPath, "a") as f:
        f.write(config)

    # Refresh terminal
    subprocess.run(["bash", "-c", f"source {bashrcPath}"], check=True)

    print("Installed Vulkan SDK for linux")

def installVulkanSDK():
    permissionGranted = False
    reply = input(f"Install Vulkan SDK {VULKAN_VERSION} (REQUIRED)? [Y/n]").lower()

    if 'n' in reply:
        return False

    # TODO: test this actually gets the correct location, even if we run some general installation script
    # Trying to find the "external" folder so we have the correct installation location
    # TODO: we'r enot using this
    scriptPath = os.path.abspath(__file__)
    scriptDir = os.path.dirname(scriptPath)
    # Where we install it
    sdkDir = os.path.join(scriptDir, "vulkansdk")

    platformName = platform.system().lower()

    # TODO: switch between windows/linux install url
    if platformName == "windows":
        # NOTE: assumes x64
        vulkanInstallerName = f"vulkansdk-{platformName}-X64-{VULKAN_VERSION}.exe"
    elif platformName == "linux":
        vulkanInstallerName = f"vulkansdk-{platformName}-x86_64-{VULKAN_VERSION}.tar.xz"

    vulkanInstallURL = f"https://sdk.lunarg.com/sdk/download/{VULKAN_VERSION}/{platformName}/{vulkanInstallerName}"
    # Install path for vulkan
    # TODO: can this be seen by CMake?
    vulkanPath = f"./external/vulkansdk/{vulkanInstallerName}"
    downloadUtils.downloadFile(vulkanInstallURL, os.path.abspath(vulkanPath))

    if platformName == "windows":
        installWindows(vulkanPath)
    elif platformName == "linux":
        # TODO: /external should be set as a global variable
        installLinux(vulkanPath, os.path.abspath("./external/vulkansdk/"))

    # TODO: delete the tarball
    
    print("Re-run to validate")
    
    return True

if __name__ == "__main__":
    if not validateVulkanSDK():
        print("Couldn't verify vulkan SDK, may cause issues")
    else:
        print("Vulkan SDK installed/validated")

