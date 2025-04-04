#!/bin/bash

#
# Global Variables used throughout the Script
#

DEVICE="" # The device you will install arch linux to

# Required because Linux appends a p for each Partition if NVME
[[ $DEVICE == *"nvme"* ]] && suffix="p"
BOOT="${DEVICE}${suffix}1"
SWAP="${DEVICE}${suffix}2"
ROOT="${DEVICE}${suffix}3"

EDITOR=""                       # nano, vi, vim, nvim, emacs, ed, etc; leave blank for none (Not recommended)
DESKTOP=""                      # KDE Plasma, Gnome, XFCE4, MATE, Cinnamon, etc; "None\" for headless/server
HOSTNAME=""                     # What your computer is known as on networks
ROOT_PASSWORD=""                # The Root account password (Leave empty to disable *hardmode*)
root_pass_set=false             # Whether or not the Root Password was set
USER_NAME=""                    # Your primary user account's name
USER_SUDO=false                 # Should your user account have sudo privileges
USER_PASSWORD=""                # The password for your user account
LOCALE=""                       # The Locale / language for your computer
REGION=""                       # The Region you exist in: US/Europe/etc.
TIMEZONE=""                     # The timezone of your computer
AUR_WRAPPER=""                  # yay, paru or "none"

#
# Helper Functions
#
archroot() {
    # User-Friendly way to issue commands from outside the chroot
    flag=$*
    arch-chroot /mnt "$flag"
}

nspawn() {
    # User-Friendly way to start services from outside the chroot
    flag=$*
    systemd-nspawn -D /mnt -- systemctl enable "$flag".service
}

catastrophic_failure() { # In the event of a catastrophic failure, display the error message
    argument=$*
    clear
    echo -e "Catastrophic Failure: ${argument}"
}

ping_fail() {
    clear
    prompt="Unfortunately an internet connection could not be established."
    prompt+="\nThe Arch installation process uses the network to download system packages."
    prompt+="\nIf you're using WiFi, please try 'iwctl station wlan0 connect <Network_SSID>'"
    prompt+="\nyou may need to change 'wlan0' to match your hardware."
    echo -e "$prompt"
}

#
# Pre-Install Tasks
# Basically just enable Parallel Downloads
parallel() {
    sed -i 's/^#ParallelDownloads/ParallelDownloads/' /etc/pacman.conf
}

multilib() {
    # Going to hack this because sed is a bitch
    cat <<EOF >> /etc/pacman.conf

[multilib]
Include = /etc/pacman.d/mirrorlist
EOF
}

unmount_all() {
    umount -R /mnt/boot/efi
    umount -R /mnt
}

#
# Actual Installation Steps taking place below here
#
init_test() { # Ensure that the host has an Internet Connection
    ping -c 1 google.com || ping_fail
    timedatectl && install
}

partition() {
    clear

    TOTAL_RAM_GB=$(free -g | awk '/^Mem:/{print $2}')
    if [[ $HYBERNATE == true ]]; then
        # Calculate swap size (1.25 times total RAM), rounding to the nearest whole number
        SWAP_SIZE="+" # Ignore this hack, please
        SWAP_SIZE+=$(awk "BEGIN {print int(($TOTAL_RAM_GB * 1.25) + 0.5)}")
        SWAP_SIZE+="G" # It works, why are you judging me?
    else
        SWAP_SIZE="+4G"
    fi

    cat <<EOF | fdisk "$DEVICE"
g
n
1

+1G
t
1
n
2

$SWAP_SIZE
t
2
swap
n
3


w
EOF

    sleep 1
}

format() {
    clear
    mkfs.fat -F 32 "$BOOT"
    mkswap "$SWAP"

    case $FILE_SYSTEM in
        "btrfs")
            mkfs.btrfs -f "$ROOT";;
        "ext4")
            yes | mkfs.ext4 "$ROOT";;
    esac
    sleep 1
}

mount_parts() {
    clear
    mount "$ROOT" /mnt
    swapon "$SWAP"
    mkdir -p /mnt/boot/efi
    mount "$BOOT" /mnt/boot/efi
    sleep 1
}

install_packages() {
    clear
    case $DESKTOP in
        "KDE Plasma") 
            packages="plasma plasma-meta konsole dolphin"
            DISPLAY_MANAGER="sddm";;
        "Gnome") 
            packages="gnome gnome-extras"
            DISPLAY_MANAGER="gdm";;
        "XFCE4")
            packages="xfce4 xfce4-goodies"
            DISPLAY_MANAGER="sddm";;
        "MATE")
            packages="mate mate-extra"
            DISPLAY_MANAGER="lightdm";;
        "Cinnamon")
            packages="cinnamon"
            DISPLAY_MANAGER="lightdm-slick-greeter";;
        "None")
            packages="";;
    esac

    case $GRAPHICS_DRIVER in
        "Nvidia Proprietary")
            drivers="nvidia-dkms egl-wayland lib32-nvidia-utils lib32-opencl-nvidia nvidia-settings opencl-nvidia nvidia-utils";;
        "Nvidia Open")
            drivers="nvidia-open-dkms egl-wayland lib32-nvidia-utils lib32-opencl-nvidia nvidia-settings opencl-nvidia nvidia-utils";;
        "vulkan-nouveau")
            drivers="vulkan-nouveau lib32-vulkan-nouveau vulkan-icd-loader lib32-vulkan-icd-loader mesa lib32-mesa";;
        "xf86-nouveau")
            drivers="xf86-video-nouveau mesa vulkan-icd-loader";;
        "Intel")
            drivers="mesa";;
        "AMD GPU")
            drivers="mesa";;
        "mesa")
            drivers="mesa";;

    esac

    pacstrap -K /mnt base base-devel linux linux-firmware linux-headers \
                     git go curl wget grub efibootmgr networkmanager \
                     "$packages" "$DISPLAY_MANAGER" "$drivers" "$EDITOR"
    sleep 1
}

generate_fstab() {
    clear
    genfstab -U /mnt >> /mnt/etc/fstab
    cat /mnt/etc/fstab
    sleep 1
}

set_rootpw() {
    clear
    echo "root:${ROOT_PASSWORD}" | archroot chpasswd
    sleep 1
}

add_user() {
    clear
    archroot useradd -m -G wheel,audio,video "$USER_NAME"
    if [[ $USER_PASSWORD != "" ]]; then
        echo "$USER_NAME:$USER_PASSWORD" | archroot chpasswd
    fi
    sleep 1
}

add_sudo() {
    clear
    cat <<EOF | arch-chroot /mnt
echo "${USER_NAME} ALL=(ALL:ALL) ALL" > /etc/sudoers.d/00_${USER_NAME}
chmod 0440 /etc/sudoers.d/00_${USER_NAME}
EOF
    sleep 1
}

systemd_services() {
    clear
    nspawn NetworkManager
    if [[ $DISPLAY_MANAGER != "" ]]; then
        nspawn "${DISPLAY_MANAGER}"
    fi
    sleep 1
}

install_bootloader() {
    clear
    archroot grub-install --target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=GRUB
    archroot grub-mkconfig -o /boot/grub/grub.cfg
    sleep 1
}

generate_locale() {
    clear
    archroot sed -i '' /etc/locale.gen # TODO: FIX
    locale-gen
    sleep 1
}

set_locale() {
    clear
    archroot echo "LANG=${LOCALE}.UTF-8" > /etc/locale.conf
    sleep 1
}

set_timezone() {
    clear
    archroot ln -sf /usr/share/zoneinfo/"${REGION}"/"${TIMEZONE}" /etc/localtime
    archroot hwclock --systohc # Sync Time / Date to new TimeZone
    sleep 1
}

install_yay() {
    export USER_NAME
    cat <<EOF > /mnt/tmp/install_yay.sh
#!/bin/bash

USER_NAME=$USER_NAME

_install_yay() {
    cd /tmp
    git clone https://aur.archlinux.org/yay.git
    chown -R \$USER_NAME yay/
    cd yay/
    su -c "makepkg" \$USER_NAME
    pacman -U --noconfirm yay-*.pkg.tar.zst
}
_install_yay
EOF
    chmod a+x /mnt/tmp/install_yay.sh
    archroot /tmp/install_yay.sh
}

install_paru() {
    export USER_NAME
    cat <<EOF > /mnt/tmp/install_paru.sh
#!/bin/bash

USER_NAME=$USER_NAME

install_paru() {
    cd /tmp
    git clone https://aur.archlinux.org/paru.git
    chown -R \$USER_NAME paru/
    cd paru/
    su -c "makepkg" \$USER_NAME
    pacman -U --noconfirm paru-*.pkg.tar.zst
}
install_paru
EOF
    chmod +x /mnt/tmp/install_paru.sh
    archroot /tmp/install_paru.sh
}

# New Install Steps
install() {
    local steps=(
        "partition:Partitioning Drive"
        "format:Formatting Partitions"
        "mount_parts:Mounting Partitions"
        "install_packages:Installing Packages"
        "generate_fstab:Generating FSTAB"
        "install_bootloader:Bootloader Installation"
        "systemd_services:SystemD Services"
        "set_rootpw:Setting Root Password"
        "add_user:Adding User"
        "generate_locale:Generating Locale"
        "set_locale:Setting Locale"
        "set_timezone:Setting Timezone"
    )

    parallel

    if [[ $GRAPHICS_DRIVER == *"nvidia"* ]]; then
        multilib
    fi

    # Run main installation steps
    for step in "${steps[@]}"; do
        IFS=":" read -r func msg <<< "$step"
        "$func" || catastrophic_failure "$msg"
    done

    if [[ $USER_SUDO == true ]]; then
        add_sudo || catastrophic_failure "Adding User to Sudoers"
    fi

    case $AUR_WRAPPER in
        "Yay") install_yay || catastrophic_failure "Installing Yay" ;;
        "Paru") install_paru || catastrophic_failure "Installing Paru" ;;
    esac
}

#
# Post Install Tasks:
#

clean_up() {
    rm -rf /mnt/tmp/*
}

#
### Debugging
#
print_to_screen() {
    prompt="DEBUGGING INSTALL CHECK"
    prompt+="\nLocale: ${SELECTED_LOCALE}" # Locale
    prompt+="\nHostname: ${NEW_HOST_NAME}" # Hostname
    prompt+="\nDevice: ${DEVICE}" # Device
    prompt+="\nHibernation: ${HIBERNATE}" # Hibernation
    prompt+="\nFile System: ${FILE_SYSTEM}" # File System
    prompt+="\nRoot Pass Set: ${root_pass_set}" # Root Password Set
    prompt+="\nRoot Password: ${ROOT_PW}" # Root Password # Insecure as hell
    prompt+="\nUser Name: ${USER_NAME}" # User account
    prompt+="\nUser Password: ${USER_PW}" # User password
    prompt+="\nUser Sudo: ${SUDOER}" # User Sudo
    prompt+="\nText Editor: ${TEXT_EDITOR}" # Text Editor
    prompt+="\nEditor Symlink: ${SYMLINK}"
    prompt+="\nDesktop: ${DESKTOP}" # Desktop
    prompt+="\nDisplay Manager: ${DISPLAY_MANAGER}" # Display Manager
    prompt+="\nGraphics Drivers: ${GRAPHICS_DRIVER}" # Graphics Drivers
    prompt+="\nDriver Packages: ${drivers}"
    prompt+="\nAUR Helper: ${AUR_WRAPPER}" # AUR Helper
    echo -e "$prompt"
}
