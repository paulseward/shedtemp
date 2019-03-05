#!/usr/bin/env bash
sudo apt-get update -y
sudo apt-get install -y prometheus
sudo cp /vagrant/prometheus.yml /etc/prometheus/prometheus.yml
sudo service prometheus restart

