#!/usr/bin/env bash
sudo apt-get update -y
sudo apt-get install -y prometheus avahi-utils
sudo cp /vagrant/prometheus.yml /etc/prometheus/prometheus.yml
sudo mv /home/vagrant/avahi-discover.sh /usr/local/bin/avahi-discover.sh
sudo mkdir -p /etc/prometheus/file_sd
echo "* * * * * root /usr/local/bin/avahi-discover.sh > /etc/prometheus/file_sd/avahi.yml" | sudo tee -a /etc/crontab
sudo /usr/local/bin/avahi-discover.sh > /etc/prometheus/file_sd/avahi.yml
sudo service prometheus restart

