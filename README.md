# Geodepot API

Geodepot helps to organize data into test cases, share them and provide integration to some of the test frameworks for ease of use. 

This repository contains the API for accessing a Geodepot repository from tests.
The API is implemented in multiple languages:

- C++ (implementation)
- Python (bindings)
- CMake (bindings)

## Command line tool (CLI)

The Geodepot API is solely meant for accessing the data in an existing Geodepot repository.
With the [Geodepot command line tool](https://github.com/3DBAG/geodepot) it is possible to create and manage a Geodepot repository.

## Documentation

https://3DBAG.github.io/geodepot

## Development

The Geodepot API code base is organized into a mono-repo.
The API has a single implementation in C++ at the root of the repository.
Implementations in other languages are bindings to the C++ code, each in their respective `geodepot-<language>` directory.

### Test data

You need [just](https://just.systems/) for downloading the data for running the tests.

```just
just download-data
```

### Testing

