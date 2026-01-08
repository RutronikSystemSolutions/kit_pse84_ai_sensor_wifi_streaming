using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace kit_pse84_ai_wifi_streaming
{
    public class NetworkStream
    {
        public enum ConnectionState
        {
            Iddle,
            Starting,
            Connected
        }

        private const byte STREAM_RADAR_HEADER = 0x1;
        private const byte STREAM_CAMERA_HEADER = 0x2;

        private const int WORKER_LOG = 1;
        private const int WORKER_CONN_STATE = 2;
        private const int WORKER_RADAR_DATA = 10;
        private const int WORKER_OV7675_DATA = 11;

        public delegate void OnNewConnectionStateEventHandler(object sender, ConnectionState state);
        public event OnNewConnectionStateEventHandler? OnNewConnectionState;

        public delegate void OnNewLogEventHandler(object sender, string log);
        public event OnNewLogEventHandler? OnNewLog;

        public delegate void OnNewRadarDataEventHandler(object sender, byte[] data);
        public event OnNewRadarDataEventHandler? OnNewRadarData;

        public delegate void OnNewCameraDataEventHandler(object sender, byte[] data);
        public event OnNewCameraDataEventHandler? OnNewCameraData;

        private BackgroundWorker? worker;

        private string? ipAddress;

        public void SetAddress(string address)
        {
            ipAddress = address;
            OnNewConnectionState?.Invoke(this, ConnectionState.Starting);
            CreateBackgroundWorker();
        }

        private void CreateBackgroundWorker()
        {
            if (worker != null)
            {
                worker.CancelAsync();
            }

            worker = new BackgroundWorker();
            worker.WorkerReportsProgress = true;
            worker.WorkerSupportsCancellation = true;
            worker.DoWork += Worker_DoWork;
            worker.ProgressChanged += Worker_ProgressChanged;
            worker.RunWorkerCompleted += Worker_RunWorkerCompleted;
            worker.RunWorkerAsync();
        }

        private void Worker_DoWork(object? sender, DoWorkEventArgs e)
        {
            if (sender == null) return;
            BackgroundWorker worker = (BackgroundWorker)sender;

            if (ipAddress == null)
            {
                worker.ReportProgress(WORKER_LOG, "Wrong ip address...");
                return;
            }

            IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse(ipAddress), 50007);
            Socket myclient = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            myclient.ReceiveTimeout = 5000;

            try
            {
                myclient.Connect(endPoint);
                worker.ReportProgress(WORKER_CONN_STATE, ConnectionState.Connected);

                for (; ; )
                {
                    // Read header
                    byte[] header = new byte[4];
                    int toRead = 4;
                    int readIndex = 0;

                    for (; ; )
                    {
                        int readFromSocket = myclient.Receive(header, readIndex, toRead, SocketFlags.None);
                        toRead -= readFromSocket;
                        readIndex += readFromSocket;
                        if (toRead == 0) break;
                    }

                    // Header read
                    if ((header[0] != 0x55) || (header[1] != 0x55))
                    {
                        worker.ReportProgress(WORKER_LOG, string.Format("wrong header! -> {0} {1}", header[0], header[1]));
                        myclient.Shutdown(SocketShutdown.Both);
                        myclient.Close();
                        break;
                    }

                    if (header[2] == STREAM_RADAR_HEADER)
                    {
                        // TODO: factorize this code
                        // +2 because first 2 bytes is frame counter
                        int radarBufferSize = ProjectConfiguration.GetRadarBufferSize() + 2;
                        byte[] buffer = new byte[radarBufferSize];
                        toRead = radarBufferSize;
                        readIndex = 0;

                        for (; ; )
                        {
                            int readFromSocket = myclient.Receive(buffer, readIndex, toRead, SocketFlags.None);
                            toRead -= readFromSocket;
                            readIndex += readFromSocket;
                            if (toRead == 0) break;
                        }

                        worker.ReportProgress(WORKER_RADAR_DATA, buffer);
                    }
                    else if (header[2] == STREAM_CAMERA_HEADER)
                    {
                        int cameraBufferSize = ProjectConfiguration.GetVideoBufferSize();
                        byte[] buffer = new byte[cameraBufferSize];
                        toRead = cameraBufferSize;
                        readIndex = 0;

                        for (; ; )
                        {
                            int readFromSocket = myclient.Receive(buffer, readIndex, toRead, SocketFlags.None);
                            toRead -= readFromSocket;
                            readIndex += readFromSocket;
                            if (toRead == 0) break;
                        }

                        worker.ReportProgress(WORKER_OV7675_DATA, buffer);
                    }
                    else
                    {
                        worker.ReportProgress(WORKER_LOG, string.Format("wrong packet type -> {0}", header[2]));
                        myclient.Shutdown(SocketShutdown.Both);
                        myclient.Close();
                        break;
                    }
                }
            }
            catch (Exception ex)
            {
                worker.ReportProgress(WORKER_LOG, ex.Message);
            }
        }

        private void Worker_ProgressChanged(object? sender, ProgressChangedEventArgs e)
        {
            switch(e.ProgressPercentage)
            {
                case WORKER_CONN_STATE:
                    if (e.UserState != null)
                    {
                        OnNewConnectionState?.Invoke(this, (ConnectionState)e.UserState);
                    }
                    break;
                case WORKER_LOG:
                    if (e.UserState != null)
                    {
                        OnNewLog?.Invoke(this, (string)e.UserState);
                    }
                    break;
                case WORKER_RADAR_DATA:
                    if (e.UserState != null)
                    {
                        OnNewRadarData?.Invoke(this, (byte[])e.UserState);
                    }
                    break;
                case WORKER_OV7675_DATA:
                    if (e.UserState != null)
                    {
                        OnNewCameraData?.Invoke(this, (byte[])e.UserState);
                    }
                    break;
            }
        }

        private void Worker_RunWorkerCompleted(object? sender, RunWorkerCompletedEventArgs e)
        {
            OnNewConnectionState?.Invoke(this, ConnectionState.Iddle);
        }
    }
}
