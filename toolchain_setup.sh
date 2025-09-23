#!/bin/bash

# To change the version, you can go to the ARM toolchain versions page,
# copy the download link, and paste it into the TOOLCHAIN_URL variable.
# You can also change the FILENAME and DIRNAME variables.
#
# Toolchain page: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

DEST="/opt"
TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz"
FILENAME="arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz"
DIRNAME="arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi"

FILEPATH="${DEST}/${FILENAME}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

#########################################################################

# Check if Python 3.8 is installed
if ! command -v python3.8 &>/dev/null; then
    echo "Python 3.8 is not installed. Proceeding with installation..."
    sudo apt update
    sudo apt upgrade -y
    sudo add-apt-repository ppa:deadsnakes/ppa -y
    sudo apt update
    sudo apt install python3.8 -y
    echo -e "${GREEN}>> Python 3.8 was installed successfully. <<${NC}"
else
    echo -e "${YELLOW}>> Warning: Python 3.8 is already installed. <<${NC}"
fi
# Check Python 3.8 version
python3.8 --version

#########################################################################

# Download the file if it does not exist
if [ ! -f "$FILEPATH" ]; then
    echo -e "${YELLOW}Downloading file...${NC}"
    if sudo wget -P "$DEST" "$TOOLCHAIN_URL"; then
        echo -e "${GREEN}>> File downloaded successfully. <<${NC}"
    else
        echo -e "${RED}>> Error: Could not download the file. Check the URL or your internet connection. <<${NC}"
        echo -e "${RED}>> The link may be expired, so you need to visit the page and update <<${NC}"
        echo -e "${RED}>> the link for the TOOLCHAIN_URL variable: <<${NC}"
        echo -e "${RED}>> Toolchain page: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads <<${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}>> Warning: The file already exists at this location: $FILEPATH <<${NC}"
fi

# Extract the file
echo "Extracting file..."
sudo tar xJf $FILEPATH -C $DEST

# Rename the folder
echo "Renaming folder..."
sudo mv "${DEST}/${DIRNAME}" "${DEST}/arm-gnu-toolchain"

# Verification
if [ -d "${DEST}/arm-gnu-toolchain" ]; then
    echo -e "${GREEN}>> Folder renamed successfully. <<${NC}"
else
   echo -e "${RED}>> Error: There was a problem renaming the folder. <<${NC}"
    exit 1
fi

#########################################################################

# Configure PATH globally
echo "Configuring PATH globally..."
CONFIG_FILE="/etc/profile.d/arm-toolchain.sh"
TOOLCHAIN_DIR="${DEST}/arm-gnu-toolchain"

# Create configuration file with export line
sudo bash -c "echo 'export PATH=${TOOLCHAIN_DIR}/bin:\$PATH' > $CONFIG_FILE"

# Verify if the file was created correctly
if [ -f "$CONFIG_FILE" ]; then
    echo -e "${GREEN}>> Configuration file created at $CONFIG_FILE. <<${NC}"
else
    echo -e "${RED}>> Error: Could not create the configuration file. <<${NC}"
    exit 1
fi

# Make the configuration file executable
echo "Making configuration file executable..."
sudo chmod +x "$CONFIG_FILE"

# Apply PATH changes globally
echo "Applying changes..."
source "$CONFIG_FILE"

# Verify if PATH was updated correctly
if [[ ":$PATH:" == *":${TOOLCHAIN_DIR}/bin:"* ]]; then
    echo -e "${GREEN}>> PATH updated successfully. Tools are available globally. <<${NC}"
else
    echo -e "${RED}>> Error: PATH was not updated correctly. <<${NC}"
    exit 1
fi

#########################################################################

# Install required libraries
echo "Installing required libraries..."
sudo apt update
sudo apt install -y libncurses-dev libtinfo-dev

# Create symbolic links for the libraries
echo "Creating symbolic links for libraries..."
sudo ln -sf /usr/lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5
sudo ln -sf /usr/lib/x86_64-linux-gnu/libncursesw.so.6 /usr/lib/x86_64-linux-gnu/libncursesw.so.5

# Verify symbolic links
if [ -L "/usr/lib/x86_64-linux-gnu/libtinfo.so.5" ] && [ -L "/usr/lib/x86_64-linux-gnu/libncursesw.so.5" ]; then
    echo -e "${GREEN}>> Symbolic links created successfully. <<${NC}"
else
    echo -e "${RED}>> Error: Could not create symbolic links. <<${NC}"
    exit 1
fi

# Verify toolchain installation
echo "Verifying toolchain installation..."
for cmd in gcc g++ gdb size; do
    if command -v arm-none-eabi-$cmd &>/dev/null; then
        echo -e "${GREEN}>> arm-none-eabi-$cmd is installed: $(arm-none-eabi-$cmd --version | head -n 1) <<${NC}"
    else
        echo -e "${RED}>> Error: arm-none-eabi-$cmd is not installed correctly. <<${NC}"
        exit 1
    fi
done

#########################################################################
# Install make
echo -e "${GREEN}Installing make...${NC}"
sudo apt install -y make

echo -e "${GREEN}Verifying make installation...${NC}"
if command -v make &>/dev/null; then
    echo -e "${GREEN}>> make is installed: $(make --version | head -n 1) <<${NC}"
else
    echo -e "${RED}>> Error: make was not installed correctly. <<${NC}"
    exit 1
fi

#########################################################################
# OpenOCD Installation
echo -e "${GREEN}Installing OpenOCD...${NC}"
sudo apt update && sudo apt install -y openocd

echo -e "${GREEN}Verifying OpenOCD installation...${NC}"
if command -v openocd &>/dev/null; then
    echo -e "${GREEN}>> OpenOCD is installed: $(openocd --version | head -n 1) <<${NC}"
else
    echo -e "${RED}>> Error: OpenOCD was not installed correctly. <<${NC}"
    exit 1
fi

#########################################################################
# Install USB utilities
echo -e "${GREEN}Installing USB utilities...${NC}"
sudo apt update && sudo apt install -y usbutils

echo -e "${GREEN}Verifying USB utilities installation...${NC}"
if command -v lsusb &>/dev/null; then
    echo -e "${GREEN}>> USB utilities installed correctly: $(lsusb --version) <<${NC}"
else
    echo -e "${RED}>> Error: USB utilities were not installed correctly. <<${NC}"
    exit 1
fi

#########################################################################

echo -e "${GREEN}>> Toolchain configured and installed successfully. <<${NC}"

# Restart the shell to apply changes
exec $SHELL
