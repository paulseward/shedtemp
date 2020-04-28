#!/bin/bash
#
# Prometheus: cron job for populating a `file_sd` service discovery file,
# based on the results of an avahi-browse query, looking for our sensors.
#
# Run this from crontab, eg:
# */5 * * * * /paul/to/avahi-discover.sh > /etc/prometheus/file_sd/avahi.yml
#
# See prometheus.yml for an example of how to add it to Prometheus config.

AVAHI_MASK="_temp-probe._tcp"
# AVAHI_MASK="-a"

## Get list of hosts
AVAHI=$(avahi-browse -rpt ${AVAHI_MASK} | awk '/^=/')
declare -A AVAHI_HOSTS

## Process hosts, one at a time
IFS=$'\n'
for line in ${AVAHI}; do
  NAME=$(echo "${line}" | awk -F';' '{print $7}')
  IP=$(echo "${line}" | awk -F';' '{print $8}')

  # Stash for later
  AVAHI_HOSTS[${NAME}]=$IP
done

## For each host, write out a prometheus host definition, including the sensor map
echo "- targets:"
for THIS in "${!AVAHI_HOSTS[@]}"; do
  echo "  - '${AVAHI_HOSTS[${THIS}]}'"
done
