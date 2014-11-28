#!/bin/bash

echo $1 > /proc/sys/net/core/rmem_default
cat /proc/sys/net/core/rmem_default > /proc/sys/net/core/rmem_max
cat /proc/sys/net/core/rmem_default > /proc/sys/net/core/wmem_max
cat /proc/sys/net/core/rmem_default > /proc/sys/net/core/wmem_default
cat /proc/sys/net/core/{r,w}mem*
