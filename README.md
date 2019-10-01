# mtc
Python module to allow OS multithreading. It currently uses POSIX threads to make a call to a python function.
The mtc funtion takes a routine as first argument and a list of lists as second. Each list will be processed in a different thread.

# Building and installing
```
python3 setup.py build
python3 setup.py install
```
