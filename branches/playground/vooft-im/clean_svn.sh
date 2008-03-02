#!/bin/sh
rm -f sim-im
find ./ -name "moc_*" -delete
find ./ -name "*.o" -delete
find ./ -name "*.so" -delete
find ./ -name "ui_*" -delete
find ./ -name "qdevelop*" -delete
find ./ -name "Makefile" -delete

