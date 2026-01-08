import arrayutils
import statistics
from scipy import signal
import numpy as np

class RadarProcessor:

    def __init__(self):
        self.antenna_count = 3
        self.chirps_per_frame = 16
        self.samples_per_chip = 128
        self.window = signal.windows.blackmanharris(self.samples_per_chip)
        self.doppler_window = signal.windows.blackmanharris(self.chirps_per_frame)

    # Return the time signal for a given antenna and a given chirp
    def get_chirp(self, raw_data, antenna_idx, chirp_idx):
        time_signal = []
        start_index = chirp_idx * self.antenna_count * self.samples_per_chip
        for i in range(self.samples_per_chip):
            index = start_index + i * self.antenna_count + antenna_idx
            time_signal.append(raw_data[index])
        return time_signal
    
    def get_max_doppler(self, raw_data):
        max_doppler = []
        for i in range(self.antenna_count):
            max_doppler.append(0)

        for antenna_idx in range(self.antenna_count):
            range_fft_array = []
            for chirp_idx in range(self.chirps_per_frame):
                # Get the time buffer
                time_buffer = self.get_chirp(raw_data, antenna_idx, chirp_idx)

                # Scale and shift
                time_buffer = arrayutils.multiply(time_buffer, 1 / 4095)
                avgValue = statistics.avg(time_buffer)
                shiftedSignal = arrayutils.apply_offset(time_buffer, -avgValue)

                for sindex in range(self.samples_per_chip):
                    shiftedSignal[sindex] = self.window[sindex] * shiftedSignal[sindex]

                # Get FFT output
                range_fft_array.append(np.fft.rfft(shiftedSignal))

            # Compute doppler FFT (1 FFT per bin)
            fft_len = int((self.samples_per_chip / 2) + 1)
            for bin_index in range(fft_len):
                bin_array = []
                for chirp_index in range(self.chirps_per_frame):
                    bin_array.append(range_fft_array[chirp_index][bin_index])

                # Remove DC
                bin_array = bin_array - (np.sum(bin_array) / len(bin_array))

                # Apply window
                bin_array = bin_array * self.doppler_window

                # Compute FFT
                fft_output = np.fft.fft(bin_array)
                fft_output = np.fft.fftshift(fft_output)

                # Check for maximum amplitude
                for i in range(len(fft_output)):
                    mag = np.abs(fft_output[i])
                    if (mag > max_doppler[antenna_idx]):
                        max_doppler[antenna_idx] = mag

        return max_doppler