using System;
using System.Collections.Generic;
using System.Text;

namespace kit_pse84_ai_wifi_streaming
{
    public class RadarProcessor
    {
        private const int samplesPerFrame = ProjectConfiguration.RADAR_ANTENNA_COUNT 
            * ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP 
            * ProjectConfiguration.RADAR_CHIRPS_PER_FRAME;

        /// <summary>
        /// Buffer used to store one frame during computation
        /// Allocated only once
        /// </summary>
        private ushort[] frame = new ushort[samplesPerFrame];

        /// <summary>
        /// Buffer used for internal computation
        /// Allocated only once
        /// </summary>
        private double[] timeBuffer = new double[ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP];

        /// <summary>
        /// Window for time signal
        /// </summary>
        private SignalWindow window = new SignalWindow(SignalWindow.Type.TypeBlackmanHarris, ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP);

        /// <summary>
        /// Window for Doppler FFT
        /// </summary>
        private SignalWindow dopplerWindow = new SignalWindow(SignalWindow.Type.TypeBlackmanHarris, ProjectConfiguration.RADAR_CHIRPS_PER_FRAME);

        /// <summary>
        /// Buffer used when computing doppler FFT for 1 bin
        /// </summary>
        private System.Numerics.Complex[] binContent = new System.Numerics.Complex[ProjectConfiguration.RADAR_CHIRPS_PER_FRAME];

        /// <summary>
        /// Store the result
        /// Updated during a call to Feed(...)
        /// </summary>
        public double[] dopplerAmplitude = new double[ProjectConfiguration.RADAR_ANTENNA_COUNT];

        /// <summary>
        /// Output size of the real FFT
        /// </summary>
        private const int spectrumLen = (ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP / 2) + 1;

        // Store matrix (range FFT per chirp)
        private System.Numerics.Complex[,] rangeFFTMatrix;

        /// <summary>
        /// Constructor
        /// </summary>
        public RadarProcessor()
        {
            rangeFFTMatrix = new System.Numerics.Complex[ProjectConfiguration.RADAR_CHIRPS_PER_FRAME, spectrumLen];
        }

        /// <summary>
        /// Feed with data
        /// First 2 bytes: frame counter
        /// Then interleaved values (depends on the number of antennas, ...)
        /// </summary>
        /// <param name="data"></param>
        public void Feed(byte[] data)
        {
            // First,convert to ushort[]
            for (int sampleIdx = 0; sampleIdx < samplesPerFrame; ++sampleIdx)
            {
                // 2 + -> because first 2 bytes of data is the frame counter
                frame[sampleIdx] = BitConverter.ToUInt16(data, 2 + sampleIdx * 2);
            }

            // Do per antenna
            for (int antennaIdx = 0; antennaIdx < ProjectConfiguration.RADAR_ANTENNA_COUNT; ++antennaIdx)
            {
                // For each chirp within a frame
                for (int chirpIx = 0; chirpIx < ProjectConfiguration.RADAR_CHIRPS_PER_FRAME; ++chirpIx)
                {
                    // Extract time signal of the chirp
                    int startIndex = chirpIx * ProjectConfiguration.RADAR_ANTENNA_COUNT * ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP;
                    for (int sampleIndex = 0; sampleIndex < ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP; sampleIndex++)
                    {
                        int index = startIndex + sampleIndex * ProjectConfiguration.RADAR_ANTENNA_COUNT + antennaIdx;
                        timeBuffer[sampleIndex] = frame[index];
                    }

                    // Time buffer now contains the samples of one antenna during a chirp
                    // Scale between 0 and 1
                    ArrayUtils.scaleInPlace(timeBuffer, 1.0 / 4095.0);

                    // Compute the average
                    double average = ArrayUtils.getAverage(timeBuffer);

                    // Offset
                    ArrayUtils.offsetInPlace(timeBuffer, -average);

                    // Apply windows
                    window.applyInPlace(timeBuffer);

                    // Compute real FFT
                    // Size of spectrum is (SamplesPerChirp / 2) + 1
                    System.Numerics.Complex[] spectrum = FftSharp.FFT.ForwardReal(timeBuffer);

                    // Store it inside the matrix (will be used to compute FFT per bin and the average)
                    for (int freqIndex = 0; freqIndex < spectrum.Length; freqIndex++)
                    {
                        rangeFFTMatrix[chirpIx, freqIndex] = spectrum[freqIndex];
                    }
                }

                // Set once
                dopplerAmplitude[antennaIdx] = 0;

                // Per bin, compute FFT and store biggest amplitude
                for (int freqIndex = 0; freqIndex < spectrumLen; freqIndex++)
                {
                    // Get the content for this frequency bin
                    for (int chirpIndex = 0; chirpIndex < ProjectConfiguration.RADAR_CHIRPS_PER_FRAME; chirpIndex++)
                    {
                        binContent[chirpIndex] = rangeFFTMatrix[chirpIndex, freqIndex];
                    }

                    // Compute average and remove it (remove 0 speed)
                    System.Numerics.Complex avgComplex = ArrayUtils.getAverage(binContent);
                    ArrayUtils.offsetInPlace(binContent, -avgComplex);

                    dopplerWindow.applyInPlaceComplex(binContent);

                    // Get FFT (transform in place)
                    FftSharp.FFT.Forward(binContent);

                    // Check for maximum value
                    for (int i = 0; i < binContent.Length; i++)
                    {
                        double magnitude = binContent[i].Magnitude;
                        if (magnitude > dopplerAmplitude[antennaIdx])
                        {
                            dopplerAmplitude[antennaIdx] = magnitude;
                        }
                    }
                }
            }
        }
    }
}
