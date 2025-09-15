#!/bin/bash

# Script to enable SSH and configure static IP in Ubuntu
# Usage: ./set_static_ip.sh [IP_ADDRESS]

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
STATIC_IP="192.168.1.59"  # Default IP
NETMASK="/24"
DNS1="8.8.8.8"
DNS2="8.8.4.4"

# If an argument is provided, use that IP
if [ ! -z "$1" ]; then
    STATIC_IP="$1"
fi

echo -e "${BLUE}=== SSH + Static IP Configurator for Ubuntu ===${NC}"
echo -e "${YELLOW}IP to configure later: ${STATIC_IP}${NC}"
echo

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}Error: This script must be run as root (use sudo)${NC}"
    exit 1
fi

# === Step 1: SSH Installation and Setup ===
echo
echo -e "${BLUE}=== Installing and Enabling SSH ===${NC}"

# Install SSH server if not present
if ! dpkg -l | grep -q openssh-server; then
    echo -e "${YELLOW}OpenSSH Server not found, installing...${NC}"
    apt update && apt install -y openssh-server
else
    echo -e "${GREEN}OpenSSH Server is already installed${NC}"
fi

# Enable and start SSH service
echo -e "${BLUE}Enabling SSH service...${NC}"
systemctl enable ssh --now

# Verify SSH status
systemctl is-active --quiet ssh
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ SSH service is running${NC}"
else
    echo -e "${RED}✗ SSH service could not be started${NC}"
    exit 1
fi

# Show current IPs before static config
echo -e "${BLUE}Current VM IP addresses:${NC}"
hostname -I
echo

# === Step 2: Static IP Configuration ===
echo -e "${BLUE}Configuring static IP...${NC}"

# Detect main network interface
INTERFACE=$(ip route | grep default | awk '{print $5}' | head -n1)

if [ -z "$INTERFACE" ]; then
    echo -e "${RED}Error: Could not detect network interface${NC}"
    exit 1
fi

echo -e "${GREEN}Interface detected: ${INTERFACE}${NC}"

# Get current gateway
GATEWAY=$(ip route | grep default | awk '{print $3}' | head -n1)

if [ -z "$GATEWAY" ]; then
    echo -e "${RED}Error: Could not detect gateway${NC}"
    exit 1
fi

echo -e "${GREEN}Gateway detected: ${GATEWAY}${NC}"

# Check if IP is available
echo -e "${BLUE}Checking availability of IP ${STATIC_IP}...${NC}"
ping -c 2 -W 2 "$STATIC_IP" > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo -e "${YELLOW}Warning: IP ${STATIC_IP} appears to be in use${NC}"
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${RED}Operation cancelled${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}IP ${STATIC_IP} is available${NC}"
fi

# Create backup of current file
NETPLAN_FILE="/etc/netplan/00-installer-config.yaml"
BACKUP_FILE="/etc/netplan/00-installer-config.yaml.backup.$(date +%Y%m%d_%H%M%S)"

if [ -f "$NETPLAN_FILE" ]; then
    echo -e "${BLUE}Creating backup: ${BACKUP_FILE}${NC}"
    cp "$NETPLAN_FILE" "$BACKUP_FILE"
fi

# Create netplan configuration
echo -e "${BLUE}Creating new network configuration...${NC}"

cat > "$NETPLAN_FILE" << EOF
network:
  version: 2
  ethernets:
    $INTERFACE:
      dhcp4: false
      addresses:
        - $STATIC_IP$NETMASK
      routes:
        - to: default
          via: $GATEWAY
      nameservers:
        addresses:
          - $DNS1
          - $DNS2
EOF

echo -e "${GREEN}Configuration created successfully${NC}"

# Apply configuration
echo -e "${BLUE}Applying network configuration...${NC}"
netplan apply

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Static IP configuration applied successfully${NC}"
else
    echo -e "${RED}✗ Error applying configuration${NC}"
    echo -e "${YELLOW}Restoring previous configuration...${NC}"
    if [ -f "$BACKUP_FILE" ]; then
        cp "$BACKUP_FILE" "$NETPLAN_FILE"
        netplan apply
        echo -e "${GREEN}Previous configuration restored${NC}"
    fi
    exit 1
fi

# Show final connection info
echo
echo -e "${BLUE}=== Final Configuration Completed ===${NC}"
echo -e "${GREEN}✓ SSH is installed and running${NC}"
echo -e "${GREEN}✓ Static IP set to: $STATIC_IP${NC}"
echo -e "${YELLOW}You can now connect using:${NC}"
echo -e "   ssh <your_user>@${STATIC_IP}"
echo
