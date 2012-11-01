# Script for debian squeeze chroot with all dependencies for QtMoko
# For the first time it installs the chroot later it just enters it

# set -o verbose

# We will use ext4 image for chroot
if [ ! -e ../qtmoko-chroot.img ]
then
    echo "Creating chroot-build ext4 image"
    dd if=/dev/zero of=../qtmoko-chroot.img bs=1024 count=3145728
    mkfs.ext4 ../qtmoko-chroot.img
fi

# Directory for chroot mount
if [ ! -d ../qtmoko-chroot ]
then
    mkdir -p ../qtmoko-chroot
fi    

# Mount the image if not mounted yet
if [ ! -d ../qtmoko-chroot/bin ]
then
    echo "Mounting image"
    mount -o loop ../qtmoko-chroot.img ../qtmoko-chroot
fi    

# Install squeeze rootfs if not installed yet
if [ ! -d ../qtmoko-chroot/bin ]
then
    if [ ! -f /usr/bin/cdebootstrap ]
    then
	echo "Installing cdebootstrap package"
	apt-get install cdebootstrap
    fi
    
    echo "Installing chroot packages"
    cdebootstrap --flavour=minimal --include=build-essential,git,openssh-client,ccache,locales,procps,psmisc,libxext-dev,libasound2-dev,libdbus-1-dev,libssl-dev,libts-dev,libbluetooth-dev,libxtst-dev,libpng12-dev,libjpeg8-dev,libv4l-dev,libspeexdsp-dev,libglib2.0-dev,libsqlite3-dev,quilt squeeze ../qtmoko-chroot http://cdn.debian.net/debian/
fi

if [ ! -d ../qtmoko-chroot/proc/bus ]
then
    echo "Mounting chroot dirs"
    mount -t proc none ../qtmoko-chroot/proc
    mount -t sysfs none ../qtmoko-chroot/sys
    mount -o bind /dev ../qtmoko-chroot/dev
    
    if [ ! -d ../qtmoko-chroot/root/qte/qtmoko ]
    then
	mkdir -p ../qtmoko-chroot/root/qte/qtmoko
	mkdir -p ../qtmoko-chroot/root/qte/build
    fi
    mount -o bind . ../qtmoko-chroot/root/qte/qtmoko
    
    echo "Build dir"
    if [ ! -d ../build ]
    then
	mkdir -p ../build
    fi
    mount -o bind ../build ../qtmoko-chroot/root/qte/build
fi

if [ ! -e ../qtmoko-chroot/usr/bin/arm-linux-gnueabi-gcc ]
then
    cat > ../qtmoko-chroot/finish_chroot_install.sh <<__END__
echo "Installing emdebian toolchain"
apt-get update
apt-get install emdebian-archive-keyring
echo "deb http://cdn.debian.net/debian squeeze main contrib non-free" > /etc/apt/sources.list
echo "deb http://www.emdebian.org/debian squeeze main" >> /etc/apt/sources.list
echo "deb http://backports.debian.org/debian-backports squeeze-backports main" >> /etc/apt/sources.list
apt-get update
apt-get install g++-4.4-arm-linux-gnueabi

echo "Installing xapt and ARM qtmoko dependencies"
apt-get install xapt
xapt -a armel -m libxext-dev libasound2-dev libdbus-1-dev libssl-dev libts-dev libbluetooth-dev libxtst-dev libpng12-dev libjpeg8-dev libv4l-dev libqt4-sql-sqlite libspeexdsp-dev libglib2.0-dev libsqlite3-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev

echo "export PATH=/usr/lib/ccache:\$PATH" >> /root/.bashrc

echo "+-----------------------------------------------------------------+"
echo "| Success! You can now build QtMoko like this:                    |"
echo "|                                                                 |"
echo "| cd /root/qte/build                                              |"
echo "| ../qtmoko/configure -build-qt -device gta04                     |"
echo "| make                                                            |"
echo "| export LD_LIBRARY_PATH=/root/qte/build/qtopiacore/host/lib/     |"
echo "| make install                                                    |"
echo "|                                                                 |"
echo "+-----------------------------------------------------------------+"

__END__
    
    chmod +x ../qtmoko-chroot/finish_chroot_install.sh
    chroot ../qtmoko-chroot /finish_chroot_install.sh
    rm -f ../qtmoko-chroot/finish_chroot_install.sh
fi

chroot ../qtmoko-chroot