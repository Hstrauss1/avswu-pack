#!/bin/sh

# Base Ubuntu system setup with SSH, VNC, and development tools

# Set user-independent variables
export TZ=America/Los_Angeles

# Update system
sudo apt-get update
sudo apt-get upgrade -y

# Install core tools
sudo apt-get install -y wget build-essential curl

# Install base system utilities
sudo apt-get install -y sudo openssh-server dbus-x11 net-tools git nano vim xfce4 --no-install-recommends xfce4-goodies

# Install Ubuntu utilities
sudo apt-get install -y zip unzip tar python3 nodejs npm xfce4-terminal tree jq dos2unix iputils-ping busybox openjdk-8-jdk

# Update system and install additional dependencies
sudo apt-get update && sudo apt-get install -y python3-pip gcc make pkg-config locales

# Install LLVM and Clang
sudo apt-get install -y lsb-release wget software-properties-common gnupg
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"

# Set system language and region
sudo locale-gen en_US.UTF-8
sudo ldconfig /usr/local/lib

# Install Node Version Manager (NVM)
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.5/install.sh | bash

# Install development tools
sudo apt-get install -y cmake clang-format

# Install browsers
sudo apt-get install -y firefox google-chrome-stable

# Install Go
GO_VERSION="1.22.0"
GO_FILE="go${GO_VERSION}.linux-amd64"
GO_URL="https://go.dev/dl/${GO_FILE}.tar.gz"
curl -L ${GO_URL} -o ${GO_FILE}.tar.gz
sudo tar -C /usr/local -xzf ${GO_FILE}.tar.gz
rm -f ${GO_FILE}.tar.gz

# Install TightVNC server and clipboard sharing
tightvncserver
sudo apt-get install -y autocutsel

# Set timezone
sudo ln -snf /usr/share/zoneinfo/$TZ /etc/localtime
sudo echo $TZ > /etc/timezone

# Start VNC server
echo "------------------------------------------------------------------"
echo "STATUS: Starting VNC server..."
echo "------------------------------------------------------------------"
touch ~/.Xauthority
mkdir -p ~/.vnc
touch ~/.Xresources
rm -rf ~/.vnc
rm -rf /tmp/.X*
tightvncserver -geometry 1920x1080
/usr/bin/autocutsel -s CLIPBOARD -fork

# Start SSH server
echo "------------------------------------------------------------------"
echo "STATUS: Starting SSH service..."
echo "------------------------------------------------------------------"
sudo service ssh start

# Set up .bash_profile
echo "------------------------------------------------------------------"
echo "STATUS: Setting up .bash_profile..."
echo "------------------------------------------------------------------"
chmod a-wx,u+rw,go+r ~/startup/_.bash_profile
cp ~/startup/_.bash_profile /etc/skel/.bash_profile
cp ~/startup/_.bash_profile ~/.bash_profile

# Copy ~/bin directory
echo "------------------------------------------------------------------"
echo "STATUS: Copying ~/bin directory..."
echo "------------------------------------------------------------------"
if [ ! -d ~/bin ]; then
    mkdir -p ~/bin
    mkdir -p /etc/skel/bin
    cp ~/startup/bin/* /etc/skel/
    cp ~/startup/bin/* ~/bin/
    cp ~/startup/bin/copy_prefix.py /usr/local/bin
    rm ~/bin/copy_prefix.py
fi

# Add user to sudoers
echo "------------------------------------------------------------------"
echo "STATUS: Adding user to sudoers file..."
echo "------------------------------------------------------------------"
sudo usermod -aG sudo $(whoami)

# Completion message
echo "------------------------------------------------------------------"
echo "STATUS: Setup Complete"
echo "------------------------------------------------------------------"
