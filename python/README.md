# Usage

- Create a virtual environment
- Install following packets:

```
matplotlib
numpy
scipy # For Blackman Harris window
opencv-python # To display picture and generate video
```

- Launch the "parse_file.py" script

# Format of the log file

```
RADAR_HEADER (0x55) or CAMERA_HEADER(0x76)
32 bits (int) of size
data
```