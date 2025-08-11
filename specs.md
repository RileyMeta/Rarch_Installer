# Basic Specs for the Project
This is an Installer (GUI) that installs a customized version of Arch Linux.
It will:
- Partition the Drive
- Format the Partitions
- Mount the Partitions
- Install Packages
- Generate FSTAB
- Set Root Password
- Configure User Account(s)
- Enable System Services
- Copy over Relevent config files
- Misc Startup script(s)

## Modes:
There are 3 Primary modes (and 2 optional) for the GUI Installer. 
- [Normal](#normal-default) - Default  (Complete Install)
- [OEM](#oem-setup) - Scripted (OEM User, second install / setup phase)
- [Recovery](#recovery) - Scripted (Leave User Account and General Files alone)
- [Fullscreen](#fullscreen) - Any Mode (Cover the entire screen [Add Simple Display Controls])
- [Debug](#debug) - Any Mode (debug to /var/rarch_installer[x].log, inc [x] by 1 if new)

### Normal (Default)
A very basic general installer that should be easy enough to (basically) guide you through the installation.<br>
Effectively have full control over everything except the packages installed and creation of the fstab (for now).<br>
Never block user-input: if they want to go back, let them. Make inputs OBVIOUS, simple names, let users clear options.

### OEM Setup
Instead of a full install, like the Default, this would work in 2 parts. The first would be a slimmed down Installer with less options outright available to the user. The second would be a "continuation" that runs on the first boot of the installed system, allowing the end-user to complete the setup.<br>
Mostly Automated, this would just be like Timezone, Networking, Disk setup and Root password, possibly even access to group policies / sudoers file directly (can be copied completely).

**Second step**<br>
Most likely the second half of the OEM setup would have to install to an (temporary) OEM account, setup and launch the OpenBox Window Manager with the second half of the OEM in Fullscreen Mode, which would accept Input for User Account name and password. After creation of the new account is confirmed; closes the installer and runs a script to: delete the temp account, uninstall BusyBox, setup the display manager and launch it for a "firt-time login".

### Recovery
Effectively a way to run the install steps individually: Re-install Bootloader, Create / Modify / Delete accounts, change Root Password, Access `arch-chroot /mnt`, etc.

### Fullscreen
A simple mode, similar to Fedora's (Anaconda) Server Installer.<br>
Meant to be used as the sole app running in a live environment with a solo window manager (OpenBox).<br>
Primarily a Fallback mode / Proof of Concept. (Might be useful for slow USBs?)

### Debug
Not a complete debug as in actively showing you everything WHILE you're doing it, but more-so a very verbose debug flag for the logs.
- [Every Log] Export time/delta from pressed to complete
- Log every button press
- Log every command used
- Log every function performed 
- Save every input (Lang, Username, Timezone, etc)
- **Do NOT save passwords**

## Unimplemented Ideas
- [ ] Basic Functionality
- [ ] Complete Functionality (Normal Only)
- [ ] Import / Export configs for easier automated (OEM) setup
- [ ] Second "step" for OEM that [Automates for OEM](#oem-setup)
- [ ] Get More Ideas
