#!/bin/sh
. /opt/qtmoko/qpe.env

if [ ! -d /dev/pts ]; then
    # mount pts so that terminal and ssh work (only needed when using devtmpfs)
    sudo mkdir -p /dev/pts
    sudo mount -t devpts none /dev/pts
    sudo modprobe joydev
fi

if [ ! "$(pidof udevd)" ]; then
  # running on devtmpfs
  wifi-poweron.sh &
fi

# Saves power in suspend
sudo rfkill block bluetooth &

# Power on modem on GTA04A4 and higher
modem-poweron.sh

sudo rm -f /dev/input/mice
echo '1' | sudo tee /proc/sys/kernel/printk
echo 0 > /sys/devices/virtual/graphics/fbcon/cursor_blink
echo timer > /sys/class/leds/gta04:green:power/trigger
echo 0 > /sys/class/leds/gta04\:red\:power/brightness
echo 0 > /sys/class/leds/gta04\:green\:power/brightness
stty -F /dev/tty1 -echo
sudo mkdir -p /var/cache/apt/archives/partial

sudo atd /var/spool/at

touch /tmp/restart-qtopia
while [ -e /tmp/restart-qtopia ]; do
    qpe
done

sudo killall -q atd
