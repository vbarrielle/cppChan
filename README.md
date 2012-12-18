# cppChan

cppChan is a C++ implementation of Go channels. Go channels are a message
passing primitive that enables easy inter-thread communication.

## Requirements

cppChan is a template library in C++11, so you'll need a compiler supporting
C++11's threading and atomic features.

## Usage

Simply include channel.hh in you project.

## Status

Channels work, but need more tests. Future developpment should add a select
functionnality to wait on multiple channels.
