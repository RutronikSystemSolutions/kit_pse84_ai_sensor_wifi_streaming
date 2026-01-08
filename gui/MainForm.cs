using System.Net;
using System.Net.Sockets;
using static System.Windows.Forms.AxHost;

namespace kit_pse84_ai_wifi_streaming
{
    public partial class MainForm : Form
    {
        private NetworkStream stream;
        private DataLogger dataLogger;
        private RadarProcessor processor = new RadarProcessor();

        public MainForm()
        {
            InitializeComponent();
            stream = new NetworkStream();
            stream.OnNewRadarData += Stream_OnNewRadarData;
            stream.OnNewCameraData += Stream_OnNewCameraData;
            stream.OnNewConnectionState += Stream_OnNewConnectionState;
            stream.OnNewLog += Stream_OnNewLog;

            dataLogger = new DataLogger();
            dataLogger.OnNewLoggerState += DataLogger_OnNewLoggerState;
            dataLogger.OnNewBytesWritten += DataLogger_OnNewBytesWritten;
        }

        private void DataLogger_OnNewBytesWritten(object sender, int bytesWritten)
        {
            string unit = "B";

            float writtenSize = (float)bytesWritten;

            if (writtenSize > (1024 * 1024))
            {
                unit = "MB";
                writtenSize = (writtenSize / (1024 * 1024));
            }
            else if (writtenSize > (1024))
            {
                unit = "kB";
                writtenSize = (writtenSize / (1024));
            }

            storedTextBox.Text = string.Format("{0:F1} {1}", writtenSize, unit);
        }

        private void DataLogger_OnNewLoggerState(object sender, DataLogger.LoggerState state)
        {
            switch (state)
            {
                case DataLogger.LoggerState.Iddle:
                    startLoggerButton.Enabled = false;
                    stopLoggerButton.Enabled = false;
                    break;

                case DataLogger.LoggerState.Ready:
                    startLoggerButton.Enabled = true;
                    stopLoggerButton.Enabled = false;
                    break;

                case DataLogger.LoggerState.Logging:
                    startLoggerButton.Enabled = false;
                    stopLoggerButton.Enabled = true;
                    break;
            }
        }

        private void Stream_OnNewLog(object sender, string log)
        {
            AddLog(log);
        }

        private void AddLog(string log)
        {
            string dateStr = DateTime.Now.ToString("HH:mm:ss");
            logTextBox.AppendText(dateStr + " > " + log + Environment.NewLine);
        }

        private void Stream_OnNewConnectionState(object sender, NetworkStream.ConnectionState state)
        {
            switch (state)
            {
                case NetworkStream.ConnectionState.Connected:
                    AddLog("Connected. Start receiving data.");
                    openConnectionButton.Enabled = false;
                    connStateTextBox.BackColor = Color.LightGreen;
                    break;
                case NetworkStream.ConnectionState.Iddle:
                    openConnectionButton.Enabled = true;
                    connStateTextBox.BackColor = Color.Coral;
                    break;
                case NetworkStream.ConnectionState.Starting:
                    connStateTextBox.BackColor = Color.LightYellow;
                    openConnectionButton.Enabled = false;
                    break;
            }
            connStateTextBox.Text = state.ToString();
        }

        private void Stream_OnNewCameraData(object sender, byte[] data)
        {
            dataLogger.LogOV7675(data);

            int width = ProjectConfiguration.VIDEO_STREAM_WIDTH_PX;
            int height = ProjectConfiguration.VIDEO_STREAM_HEIGHT_PX;

            var u16buffer = new ushort[(width * height)];
            for (int i = 0; i < u16buffer.Length; ++i)
            {
                u16buffer[i] = BitConverter.ToUInt16(data, i * 2);
            }

            Bitmap bmp = new Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format24bppRgb);
            for (int x = 0; x < width; ++x)
            {
                for (int y = 0; y < height; ++y)
                {
                    var p = u16buffer[y * width + x];

                    int r = ((p >> 11) & 0x1f) << 3;
                    int g = ((p >> 5) & 0x3f) << 2;
                    int b = ((p >> 0) & 0x1f) << 3;

                    Color color = Color.FromArgb(r, g, b);
                    bmp.SetPixel(x, y, color);
                }
            }

            cameraPictureBox.Image = bmp;
        }

        private void Stream_OnNewRadarData(object sender, byte[] data)
        {
            dataLogger.LogRadar(data);

            // Extract only the first chirp
            int firstChirpSampleCount = ProjectConfiguration.RADAR_ANTENNA_COUNT * ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP;
            double[] firstChirpSamples = new double[firstChirpSampleCount];

            ushort frameIndex = BitConverter.ToUInt16(data, 0);

            for (int sampleIdx = 0; sampleIdx < firstChirpSampleCount; ++sampleIdx)
            {
                // 2 + -> because first 2 bytes of data is the frame counter
                firstChirpSamples[sampleIdx] = BitConverter.ToUInt16(data, 2 + sampleIdx * 2);
                firstChirpSamples[sampleIdx] = firstChirpSamples[sampleIdx] / 4095;
            }

            processor.Feed(data);

            amplitudeTimeView.updateData(frameIndex, processor.dopplerAmplitude);
            timeSignalView.updateData(firstChirpSamples);
        }

        private void openConnectionButton_Click(object sender, EventArgs e)
        {
            AddLog("Try to open connection with 192.168.10.1");
            stream.SetAddress("192.168.10.1");
            openConnectionButton.Enabled = false;
        }

        private void selectPathButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "Binary Log file|*.log";
            sfd.Title = "Save stream to file";
            if (sfd.ShowDialog() != DialogResult.OK) return;

            filePathTextBox.Text = sfd.FileName;
            dataLogger.SetPath(sfd.FileName);
        }

        private void startLoggerButton_Click(object sender, EventArgs e)
        {
            dataLogger.Start();
            startLoggerButton.Enabled = false;
        }

        private void stopLoggerButton_Click(object sender, EventArgs e)
        {
            dataLogger.Stop();
            stopLoggerButton.Enabled = false;
        }
    }
}
