# ESP8266 based Temperature sensing for my shed

## Hardware:
esp-01 + DS18B20 - prebuilt boards are available for a couple of quid that make deploying sensors easy.

## Software - Probe:
The arduino sketch in this repo makes temperature readings available via a Prometheus metrics endpoint.  This allows metrics to be easily "pulled" from an arbitrary number of sensors.

Metrics dataformat is as per: https://prometheus.io/docs/instrumenting/exposition_formats/

The arduino sketch can support multiple DS18B20 sensors on the same I2C bus, just connect them in parallel.

Each temperature probe is exposed using its ID as a unique identifier.

## Software - Prometheus:

### Service Discovery:
The temperature probes will announce themselves on the local network via MDNS, using the service tag `_temp-probe._tcp`

There is a shell script included in the `prometheus-discovery` folder, which uses `avahi-browse` on linux to discover probes, and build a file describing the probes it's found.  This file can be used with Prometheus `file_sd` to provide service discovery.

See the Prometheus documentation for more info: https://prometheus.io/docs/guides/file-sd/

### Testing
The `vagrant` folder contains a simple vagrant environment that brings up prometheus, with avahi based discovery.

To use this, install vagrant (https://www.vagrantup.com/) then `cd` into the `vagrant` folder, and run `vagrant up` service discovery runs every minute.
