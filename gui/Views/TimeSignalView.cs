using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.Series;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace kit_pse84_ai_wifi_streaming.Views
{
    public partial class TimeSignalView : UserControl
    {
        /// <summary>
        /// X Axis
        /// </summary>
        LinearAxis xAxis = new LinearAxis
        {
            MajorGridlineStyle = LineStyle.Dot,
            Position = AxisPosition.Bottom,
            AxislineStyle = LineStyle.Solid,
            AxislineColor = OxyColors.Gray,
            FontSize = 10,
            PositionAtZeroCrossing = true,
            IsPanEnabled = false,
            IsZoomEnabled = true,
            Unit = "Sample index"
        };

        /// <summary>
        /// Y Axis for amplitude
        /// </summary>
        private LinearAxis yAxis = new LinearAxis
        {
            MajorGridlineStyle = LineStyle.Dot,
            AxislineStyle = LineStyle.Solid,
            AxislineColor = OxyColors.Gray,
            FontSize = 10,
            TextColor = OxyColors.Gray,
            Position = AxisPosition.Left,
            IsPanEnabled = false,
            IsZoomEnabled = true,
            Minimum = 0,
            Maximum = 1,
            Unit = "ADC tick",
            Key = "Amp",
        };

        private List<LineSeries> lines = new List<LineSeries>();

        public TimeSignalView()
        {
            InitializeComponent();
            InitPlot();
        }

        private void InitPlot()
        {
            // Raw signals plot
            var timeModel = new PlotModel
            {
                PlotType = PlotType.XY,
                PlotAreaBorderThickness = new OxyThickness(0),
            };

            // Set the axes
            timeModel.Axes.Add(xAxis);
            timeModel.Axes.Add(yAxis);

            // As many line series as antennas
            lines.Clear();
            for(int antennaIndex = 0; antennaIndex < ProjectConfiguration.RADAR_ANTENNA_COUNT; ++antennaIndex)
            {
                LineSeries series = new LineSeries();
                series.Title = string.Format("Antenna {0}", antennaIndex);
                series.YAxisKey = yAxis.Key;
                lines.Add(series);
                timeModel.Series.Add(series);
            }

            plotView.Model = timeModel;
            plotView.InvalidatePlot(true);
        }

        /// <summary>
        /// Set time signal
        /// /!\ Signal is interleaved:
        /// - 0: antenna 0 sample 0
        /// - 1: antenna 1 sample 0
        /// - 2: antenna 2 sample 0
        /// - 3: antenna 0 sample 1
        /// - ...
        /// </summary>
        /// <param name="signal"></param>
        public void updateData(double[] signal)
        {
            for(int antennaIndex = 0; antennaIndex < ProjectConfiguration.RADAR_ANTENNA_COUNT; ++antennaIndex)
            {
                lines[antennaIndex].Points.Clear();
                for (int sampleIdx = 0; sampleIdx < ProjectConfiguration.RADAR_SAMPLES_PER_CHIRP; ++sampleIdx)
                {
                    int index = antennaIndex + (sampleIdx * ProjectConfiguration.RADAR_ANTENNA_COUNT);
                    lines[antennaIndex].Points.Add(new DataPoint(sampleIdx, signal[index]));
                }
            }
            plotView.InvalidatePlot(true);
        }
    }
}
