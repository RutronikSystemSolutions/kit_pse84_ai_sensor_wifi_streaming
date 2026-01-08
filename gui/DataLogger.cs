using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace kit_pse84_ai_wifi_streaming
{
    public class DataLogger
    {
        public enum LoggerState
        {
            Iddle,
            Ready,
            Logging
        }

        public delegate void OnNewLoggerStateEventHandler(object sender, LoggerState state);
        public event OnNewLoggerStateEventHandler? OnNewLoggerState;

        public delegate void OnNewBytesWrittenEventHandler(object sender, int bytesWritten);
        public event OnNewBytesWrittenEventHandler? OnNewBytesWritten;

        private bool logging = false;

        private string? path;

        private List<byte[]> ov7675Packets = new List<byte[]>();
        private List<byte[]> radarPackets = new List<byte[]>();
        private object sync = new object();

        private const int BYTES_WRITTEN = 10;

        private const int MAX_CACHE_COUNT = 10;

        private const byte RADAR_HEADER = 0x55;
        private const byte OV7675_HEADER = 0x76;

        /// <summary>
        /// Background worker enabling background operations
        /// </summary>
        private BackgroundWorker? worker;

        private void CreateBackgroundWorkerAndStart()
        {
            if (this.worker != null)
            {
                Stop();
            }

            this.worker = new BackgroundWorker();
            this.worker.WorkerReportsProgress = true;
            this.worker.WorkerSupportsCancellation = true;
            this.worker.DoWork += Worker_DoWork;
            this.worker.ProgressChanged += Worker_ProgressChanged;
            this.worker.RunWorkerAsync();
        }

        private void Worker_ProgressChanged(object? sender, ProgressChangedEventArgs e)
        {
            switch (e.ProgressPercentage)
            {
                case BYTES_WRITTEN:
                    if (e.UserState != null)
                    {
                        OnNewBytesWritten?.Invoke(this, (int)e.UserState);
                    }
                    break;
            }
        }

        public void SetPath(string path)
        {
            if (logging) return;

            this.path = path;
            OnNewLoggerState?.Invoke(this, LoggerState.Ready);
        }

        public void Start()
        {
            if (logging) return;

            CreateBackgroundWorkerAndStart();

            logging = true;

            OnNewLoggerState?.Invoke(this, LoggerState.Logging);
        }

        /// <summary>
        /// Stop to log the data
        /// </summary>
        public void Stop()
        {
            if (this.worker == null) return;

            this.worker.CancelAsync();
            this.worker.DoWork -= Worker_DoWork;
            this.worker.ProgressChanged -= Worker_ProgressChanged;

            logging = false;

            OnNewLoggerState?.Invoke(this, LoggerState.Ready);
        }

        public void LogOV7675(byte[] packet)
        {
            if (logging == false) return;

            lock (sync)
            {
                if (ov7675Packets.Count > MAX_CACHE_COUNT)
                {
                    System.Diagnostics.Debug.WriteLine("too much ov7675");
                }
                else
                {
                    ov7675Packets.Add(packet);
                }
            }
        }

        public void LogRadar(byte[] packet)
        {
            if (logging == false) return;

            lock (sync)
            {
                if (radarPackets.Count > MAX_CACHE_COUNT)
                {
                    System.Diagnostics.Debug.WriteLine("too much radar");
                }
                else
                {
                    radarPackets.Add(packet);
                }
            }
        }

        private void Worker_DoWork(object? sender, DoWorkEventArgs e)
        {
            if (sender == null) return;
            if (path == null) return;

            BackgroundWorker? worker = (BackgroundWorker)sender;
            if (worker == null) return;

            var stream = File.Open(path, FileMode.Create);
            BinaryWriter writer = new BinaryWriter(stream);

            int bytesWritten = 0;

            for (; ; )
            {
                if (worker.CancellationPending)
                {
                    lock (sync)
                    {
                        ov7675Packets.Clear();
                        radarPackets.Clear();
                    }
                    writer.Close();
                    return;
                }

                bool dataAvailable = false;

                byte[]? ov7675data = null;
                lock (sync)
                {
                    if (ov7675Packets.Count > 0)
                    {
                        ov7675data = ov7675Packets[0];
                        ov7675Packets.RemoveAt(0);
                        dataAvailable = true;
                    }
                }

                if (ov7675data != null)
                {
                    int size = ov7675data.Length;
                    writer.Write(OV7675_HEADER);
                    writer.Write(size);
                    writer.Write(ov7675data);
                    bytesWritten += ov7675data.Length;
                }

                byte[]? radarData = null;
                lock (sync)
                {
                    if (radarPackets.Count > 0)
                    {
                        radarData = radarPackets[0];
                        radarPackets.RemoveAt(0);
                        dataAvailable = true;
                    }
                }

                if (radarData != null)
                {
                    int size = radarData.Length;
                    writer.Write(RADAR_HEADER);
                    writer.Write(size);
                    writer.Write(radarData);
                    bytesWritten += radarData.Length;
                }

                if (dataAvailable == false)
                {
                    System.Threading.Thread.Sleep(50);
                }
                else
                {
                    worker.ReportProgress(BYTES_WRITTEN, bytesWritten);
                }
            }
        }
    }
}

