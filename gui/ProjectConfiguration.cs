using System;
using System.Collections.Generic;
using System.Text;

namespace kit_pse84_ai_wifi_streaming
{
    public class ProjectConfiguration
    {
        public const int VIDEO_STREAM_WIDTH_PX = 320;
        public const int VIDEO_STREAM_HEIGHT_PX = 240;
        public const int VIDEO_STREAM_BYTES_PER_PIXEL = 2;

        public const int RADAR_ANTENNA_COUNT = 3;
        public const int RADAR_SAMPLES_PER_CHIRP = 128;
        public const int RADAR_CHIRPS_PER_FRAME = 16;
        public const int RADAR_BYTES_PER_SAMPLE = 2;

        public static int GetRadarBufferSize()
        {
            return RADAR_ANTENNA_COUNT * RADAR_SAMPLES_PER_CHIRP * RADAR_CHIRPS_PER_FRAME * RADAR_BYTES_PER_SAMPLE;
        }

        public static int GetVideoBufferSize()
        {
            return VIDEO_STREAM_WIDTH_PX * VIDEO_STREAM_HEIGHT_PX * VIDEO_STREAM_BYTES_PER_PIXEL;
        }
    }
}
